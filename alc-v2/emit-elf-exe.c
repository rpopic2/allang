#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "types.h"
#include "emit.h"
#include "err.h"
#include "emit-bin.h"

typedef struct {
    u8 e_ident[16];
    u16 e_type;
    u16 e_machine;
    u32 e_version;
    u64 e_entry;
    u64 e_phoff;
    u64 e_shoff;
    u32 e_flags;
    u16 e_ehsize;
    u16 e_phentsize;
    u16 e_phnum;
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
} elf64_ehdr;

typedef struct {
    u32 p_type;
    u32 p_flags;
    u64 p_offset;
    u64 p_vaddr;
    u64 p_paddr;
    u64 p_filesz;
    u64 p_memsz;
    u64 p_align;
} elf64_phdr;

_Static_assert(sizeof(elf64_ehdr) == 64, "elf64_ehdr must be 64 bytes");
_Static_assert(sizeof(elf64_phdr) == 56, "elf64_phdr must be 56 bytes");

#define PT_LOAD    1u
#define PF_X       1u
#define PF_W       2u
#define PF_R       4u
#define ET_EXEC    2u
#define EM_X86_64  62u
#define LOAD_BASE  0x400000UL
#define PAGE       0x1000UL

/* _start: call main; mov edi, eax; mov eax, 60; syscall */
#define START_STUB_SIZE 14u
#define START_CALL_DISP_OFF 1u

static void build_start_stub(u8 *p, int32_t call_rel) {
    p[0] = 0xe8;
    memcpy(p + START_CALL_DISP_OFF, &call_rel, 4);
    p[5] = 0x89;
    p[6] = 0xc7;
    p[7] = 0xb8;
    p[8] = 60;
    p[9] = 0;
    p[10] = 0;
    p[11] = 0;
    p[12] = 0x0f;
    p[13] = 0x05;
}

static void write_static_elf(FILE *out, const bin_image *image) {
    static u8 img[0x80000];

    const u64 headers_size = sizeof(elf64_ehdr) + sizeof(elf64_phdr);
    const u64 stub_off = headers_size;
    const u64 text_off = stub_off + START_STUB_SIZE;
    const u64 text_vaddr = LOAD_BASE + text_off;
    const u64 image_size = text_off + image->text_size;

    if (image_size > sizeof img) {
        report_error("elf-exe: image too large\n");
        return;
    }

    const elf64_ehdr ehdr = {
        .e_ident = { 0x7f, 'E', 'L', 'F', 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        .e_type = ET_EXEC,
        .e_machine = EM_X86_64,
        .e_version = 1,
        .e_entry = LOAD_BASE + stub_off,
        .e_phoff = sizeof(elf64_ehdr),
        .e_ehsize = (u16)sizeof(elf64_ehdr),
        .e_phentsize = (u16)sizeof(elf64_phdr),
        .e_phnum = 1,
    };

    const elf64_phdr phdr = {
        .p_type = PT_LOAD,
        .p_flags = PF_R | PF_W | PF_X,
        .p_offset = 0,
        .p_vaddr = LOAD_BASE,
        .p_paddr = LOAD_BASE,
        .p_filesz = image_size,
        .p_memsz = image_size,
        .p_align = PAGE,
    };

    memcpy(img, &ehdr, sizeof ehdr);
    memcpy(img + sizeof ehdr, &phdr, sizeof phdr);

    u64 main_vaddr = text_vaddr + image->entry;
    u64 call_next = LOAD_BASE + stub_off + START_CALL_DISP_OFF + 4;
    int32_t call_rel = (int32_t)((i64)main_vaddr - (i64)call_next);
    build_start_stub(img + stub_off, call_rel);

    memcpy(img + text_off, image->text, image->text_size);

    fwrite(img, 1, image_size, out);
    fflush(out);
    fchmod(fileno(out), 0755);
}

void emit_init(void) {}

void emit_output(FILE *out) {
    bin_image image;
    bin_emit(&image);
    if (image.imports_count > 0) {
        report_error("elf-exe: dynamic imports not yet supported (%u)\n", image.imports_count);
        return;
    }
    write_static_elf(out, &image);
}

const char *text_section_header = "";
const char *string_section_header = "";
const char *output_ext = "";
