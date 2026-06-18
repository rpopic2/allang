#pragma once

#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include "emit.h"
#include "err.h"
#include "emit-bin.h"

#define PE_IMAGE_BASE     0x140000000ULL
#define PE_SECTION_ALIGN  0x1000u
#define PE_FILE_ALIGN     0x200u
#define PE_NUM_SECTIONS   2u
#define PE_IMPORT_DLL     "msvcrt.dll"
#define PE_EXIT_NAME      "exit"
#define PE_MAX_IMPORTS    128u
#define PE_THUNK_ENTRY    8u

_Alignas(16) static uint8_t img[0x80000];

static uint32_t pe_align(uint32_t x, uint32_t a) {
    return (x + a - 1) & ~(a - 1);
}

static const char *pe_export_name(const char *name) {
    if (strcmp(name, "_Exit") == 0)
        return "_exit";
    return name;
}

static const char *pe_import_name(const bin_image *image, uint32_t i, uint32_t exit_idx) {
    if (i == exit_idx)
        return PE_EXIT_NAME;
    return pe_export_name(image->imports[i].name);
}

static void write_pe(FILE *out, const bin_image *image) {
    const uint32_t nuser = image->imports_count;
    const uint32_t nimp = nuser + 1;
    const uint32_t exit_idx = nuser;

    const uint32_t nt_off = (uint32_t)sizeof(IMAGE_DOS_HEADER);
    const uint32_t sec_off = nt_off + (uint32_t)sizeof(IMAGE_NT_HEADERS64);
    const uint32_t headers_end = sec_off + PE_NUM_SECTIONS * (uint32_t)sizeof(IMAGE_SECTION_HEADER);
    const uint32_t size_of_headers = pe_align(headers_end, PE_FILE_ALIGN);

    const uint32_t text_va = PE_SECTION_ALIGN;
    const uint32_t stub_rva = text_va;
    const uint32_t thunks_rva = stub_rva + pe_entry_stub_size;
    const uint32_t code_rva = thunks_rva + nimp * pe_thunk_size;
    const uint32_t text_vsize = (code_rva - text_va) + (uint32_t)image->text_size;
    const uint32_t text_raw_ptr = size_of_headers;
    const uint32_t text_raw_size = pe_align(text_vsize, PE_FILE_ALIGN);

    const uint32_t idata_va = pe_align(text_va + text_vsize, PE_SECTION_ALIGN);
    const uint32_t desc_off = 0;
    const uint32_t desc_size = 2u * (uint32_t)sizeof(IMAGE_IMPORT_DESCRIPTOR);
    const uint32_t ilt_off = desc_off + desc_size;
    const uint32_t ilt_size = (nimp + 1) * PE_THUNK_ENTRY;
    const uint32_t iat_off = ilt_off + ilt_size;
    const uint32_t iat_size = (nimp + 1) * PE_THUNK_ENTRY;

    static uint32_t name_off[PE_MAX_IMPORTS];
    if (nimp > PE_MAX_IMPORTS) {
        report_err("pe-exe: too many imports\n");
        return;
    }
    uint32_t cur = iat_off + iat_size;
    for (uint32_t i = 0; i < nimp; i++) {
        name_off[i] = cur;
        const char *nm = pe_import_name(image, i, exit_idx);
        cur += pe_align(2u + (uint32_t)strlen(nm) + 1u, 2u);
    }
    const uint32_t dllname_off = cur;
    cur += (uint32_t)strlen(PE_IMPORT_DLL) + 1u;

    const uint32_t idata_vsize = cur;
    const uint32_t idata_raw_ptr = text_raw_ptr + text_raw_size;
    const uint32_t idata_raw_size = pe_align(idata_vsize, PE_FILE_ALIGN);
    const uint32_t image_size = pe_align(idata_va + idata_vsize, PE_SECTION_ALIGN);
    const uint32_t total_file = idata_raw_ptr + idata_raw_size;

    if (total_file > sizeof img) {
        report_err("pe-exe: image too large\n");
        return;
    }
    memset(img, 0, total_file);

    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)img;
    dos->e_magic = IMAGE_DOS_SIGNATURE;
    dos->e_lfanew = (LONG)nt_off;

    IMAGE_NT_HEADERS64 *nt = (IMAGE_NT_HEADERS64 *)(img + nt_off);
    nt->Signature = IMAGE_NT_SIGNATURE;
    nt->FileHeader.Machine = pe_machine;
    nt->FileHeader.NumberOfSections = (WORD)PE_NUM_SECTIONS;
    nt->FileHeader.SizeOfOptionalHeader = (WORD)sizeof(IMAGE_OPTIONAL_HEADER64);
    nt->FileHeader.Characteristics =
        (WORD)(IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LARGE_ADDRESS_AWARE);

    IMAGE_OPTIONAL_HEADER64 *opt = &nt->OptionalHeader;
    opt->Magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
    opt->AddressOfEntryPoint = stub_rva;
    opt->BaseOfCode = text_va;
    opt->ImageBase = PE_IMAGE_BASE;
    opt->SectionAlignment = PE_SECTION_ALIGN;
    opt->FileAlignment = PE_FILE_ALIGN;
    opt->MajorOperatingSystemVersion = (WORD)6;
    opt->MajorSubsystemVersion = (WORD)6;
    opt->SizeOfImage = image_size;
    opt->SizeOfHeaders = size_of_headers;
    opt->Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
    opt->SizeOfStackReserve = 0x100000ULL;
    opt->SizeOfStackCommit = 0x1000ULL;
    opt->SizeOfHeapReserve = 0x100000ULL;
    opt->SizeOfHeapCommit = 0x1000ULL;
    opt->NumberOfRvaAndSizes = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
    opt->SizeOfCode = text_raw_size;
    opt->SizeOfInitializedData = idata_raw_size;
    opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = idata_va + desc_off;
    opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = desc_size;
    opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress = idata_va + iat_off;
    opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size = iat_size;

    IMAGE_SECTION_HEADER *sec = (IMAGE_SECTION_HEADER *)(img + sec_off);
    memcpy(sec[0].Name, ".text", sizeof ".text" - 1);
    sec[0].Misc.VirtualSize = text_vsize;
    sec[0].VirtualAddress = text_va;
    sec[0].SizeOfRawData = text_raw_size;
    sec[0].PointerToRawData = text_raw_ptr;
    sec[0].Characteristics =
        IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;
    memcpy(sec[1].Name, ".idata", sizeof ".idata" - 1);
    sec[1].Misc.VirtualSize = idata_vsize;
    sec[1].VirtualAddress = idata_va;
    sec[1].SizeOfRawData = idata_raw_size;
    sec[1].PointerToRawData = idata_raw_ptr;
    sec[1].Characteristics =
        IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;

    uint8_t *const text = img + text_raw_ptr;
    const uint32_t main_rva = code_rva + image->entry;
    const uint32_t exit_thunk_rva = thunks_rva + exit_idx * pe_thunk_size;
    pe_build_entry_stub(text, stub_rva, main_rva, exit_thunk_rva);
    for (uint32_t i = 0; i < nimp; i++) {
        uint32_t trva = thunks_rva + i * pe_thunk_size;
        uint32_t islot = idata_va + iat_off + i * PE_THUNK_ENTRY;
        pe_build_thunk(text + (trva - text_va), trva, islot);
    }
    memcpy(text + (code_rva - text_va), image->text, image->text_size);

    for (uint32_t i = 0; i < image->extcalls_count; i++) {
        const bin_extcall *e = &image->extcalls[i];
        uint32_t site_rva = code_rva + e->site;
        uint32_t trva = thunks_rva + e->import * pe_thunk_size;
        pe_patch_call(text + (code_rva - text_va) + e->site, site_rva, trva);
    }

    uint8_t *const idata = img + idata_raw_ptr;
    IMAGE_IMPORT_DESCRIPTOR *desc = (IMAGE_IMPORT_DESCRIPTOR *)(idata + desc_off);
    desc[0].OriginalFirstThunk = idata_va + ilt_off;
    desc[0].Name = idata_va + dllname_off;
    desc[0].FirstThunk = idata_va + iat_off;

    for (uint32_t i = 0; i < nimp; i++) {
        uint64_t entry = idata_va + name_off[i];
        memcpy(idata + ilt_off + i * PE_THUNK_ENTRY, &entry, PE_THUNK_ENTRY);
        memcpy(idata + iat_off + i * PE_THUNK_ENTRY, &entry, PE_THUNK_ENTRY);
    }

    for (uint32_t i = 0; i < nimp; i++) {
        const char *nm = pe_import_name(image, i, exit_idx);
        memcpy(idata + name_off[i] + 2u, nm, strlen(nm) + 1);
    }
    memcpy(idata + dllname_off, PE_IMPORT_DLL, strlen(PE_IMPORT_DLL) + 1);

    _setmode(_fileno(out), _O_BINARY);
    fwrite(img, 1, total_file, out);
    fflush(out);
}

void emit_init(void) {}

void emit_output(FILE *out) {
    bin_image image;
    bin_emit(&image);
    write_pe(out, &image);
}

const char *text_section_header = "";
const char *string_section_header = "";
const char *output_ext = "exe";
