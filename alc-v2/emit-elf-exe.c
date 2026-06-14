#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <elf.h>
#include <sys/stat.h>
#include <unistd.h>

#include "types.h"
#include "emit.h"
#include "err.h"
#include "emit-bin.h"

#define LOAD_BASE  0x400000UL
#define PAGE       0x1000UL
#define INTERP     "/lib64/ld-linux-x86-64.so.2"
#define LIBNAME    "libc.so.6"

/* static _start: call main; mov edi, eax; mov eax, 60; syscall */
#define START_STUB_SIZE     14u
#define START_CALL_DISP_OFF 1u
/* dynamic _start: call main; mov edi, eax; call exit@plt */
#define DYN_START_STUB_SIZE 12u
/* standard lazy PLT: PLT[0] is the resolver stub, PLT[n] are 16-byte entries */
#define PLT_ENTRY_SIZE      16u
/* GOT[0]=.dynamic, GOT[1]=link-map(ld.so), GOT[2]=resolver(ld.so) */
#define GOT_RESERVED        3u

static u8 img[0x80000];

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

static void build_dyn_start_stub(u8 *p, int32_t call_main_rel, int32_t call_exit_rel) {
    p[0] = 0xe8;
    memcpy(p + 1, &call_main_rel, 4);
    p[5] = 0x89;
    p[6] = 0xc7;
    p[7] = 0xe8;
    memcpy(p + 8, &call_exit_rel, 4);
}

static u64 align_up(u64 x, u64 a) {
    return (x + a - 1) & ~(a - 1);
}

static void patch_extcalls(const bin_image *image, u64 text_off, u64 plt_off) {
    for (u32 i = 0; i < image->extcalls_count; i++) {
        const bin_extcall *e = &image->extcalls[i];
        /* PLT[0] is the resolver stub; user imports start at PLT[1] */
        u64 stub_vaddr = LOAD_BASE + plt_off + (u64)(e->import + 1) * PLT_ENTRY_SIZE;
        u64 call_next = LOAD_BASE + text_off + e->site + 4;
        int32_t rel = (int32_t)((i64)stub_vaddr - (i64)call_next);
        memcpy(img + text_off + e->site, &rel, 4);
    }
}

static void finish(FILE *out, u64 size) {
    fwrite(img, 1, size, out);
    fflush(out);
    fchmod(fileno(out), 0755);
}

static void write_static_elf(FILE *out, const bin_image *image) {
    const u64 headers_size = sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr);
    const u64 stub_off = headers_size;
    const u64 text_off = stub_off + START_STUB_SIZE;
    const u64 text_vaddr = LOAD_BASE + text_off;
    const u64 image_size = text_off + image->text_size;

    if (image_size > sizeof img) {
        report_error("elf-exe: image too large\n");
        return;
    }
    memset(img, 0, image_size);

    Elf64_Ehdr *eh = (Elf64_Ehdr *)img;
    eh->e_ident[EI_MAG0] = ELFMAG0;
    eh->e_ident[EI_MAG1] = ELFMAG1;
    eh->e_ident[EI_MAG2] = ELFMAG2;
    eh->e_ident[EI_MAG3] = ELFMAG3;
    eh->e_ident[EI_CLASS] = ELFCLASS64;
    eh->e_ident[EI_DATA] = ELFDATA2LSB;
    eh->e_ident[EI_VERSION] = EV_CURRENT;
    eh->e_type = ET_EXEC;
    eh->e_machine = EM_X86_64;
    eh->e_version = EV_CURRENT;
    eh->e_entry = LOAD_BASE + stub_off;
    eh->e_phoff = sizeof(Elf64_Ehdr);
    eh->e_ehsize = sizeof(Elf64_Ehdr);
    eh->e_phentsize = sizeof(Elf64_Phdr);
    eh->e_phnum = 1;

    Elf64_Phdr *ph = (Elf64_Phdr *)(img + sizeof(Elf64_Ehdr));
    ph->p_type = PT_LOAD;
    ph->p_flags = PF_R | PF_W | PF_X;
    ph->p_offset = 0;
    ph->p_vaddr = LOAD_BASE;
    ph->p_paddr = LOAD_BASE;
    ph->p_filesz = image_size;
    ph->p_memsz = image_size;
    ph->p_align = PAGE;

    u64 main_vaddr = text_vaddr + image->entry;
    u64 call_next = LOAD_BASE + stub_off + START_CALL_DISP_OFF + 4;
    int32_t call_rel = (int32_t)((i64)main_vaddr - (i64)call_next);
    build_start_stub(img + stub_off, call_rel);

    memcpy(img + text_off, image->text, image->text_size);
    finish(out, image_size);
}

static u32 elf_hash(const char *name) {
    u32 h = 0;
    while (*name) {
        h = (h << 4) + (u8)*name++;
        u32 g = h & 0xf0000000u;
        if (g)
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

/* the program's imports plus a synthetic trailing "exit" used by _start to
   flush libc stdio buffers before terminating. */
static const char *import_name(const bin_image *image, u32 i) {
    if (i < image->imports_count)
        return image->imports[i].name;
    return "exit";
}

static void write_dynamic_elf(FILE *out, const bin_image *image) {
    const u32 exit_idx = image->imports_count;
    const u32 nimp = image->imports_count + 1;
    const u32 nsym = nimp + 1;
    const u32 nbucket = nimp;
    /* PLT[0]=resolver + PLT[1..nimp] per import; GOT has 3 reserved + nimp entries */
    const u32 nplt = nimp + 1;
    const u32 ndyn = 11;
    const int nphdr = 5;

    /* dynstr: '\0' + LIBNAME + import names */
    static char dynstr[0x2000];
    u32 dynstr_len = 0;
    dynstr[dynstr_len++] = '\0';
    u32 lib_name_off = dynstr_len;
    memcpy(dynstr + dynstr_len, LIBNAME, sizeof LIBNAME);
    dynstr_len += (u32)sizeof LIBNAME;
    static u32 sym_name_off[64];
    for (u32 i = 0; i < nimp; i++) {
        sym_name_off[i] = dynstr_len;
        size_t l = strlen(import_name(image, i)) + 1;
        memcpy(dynstr + dynstr_len, import_name(image, i), l);
        dynstr_len += (u32)l;
    }

    const u64 ehdr_off = 0;
    const u64 phdr_off = sizeof(Elf64_Ehdr);
    u64 cur = phdr_off + (u64)nphdr * sizeof(Elf64_Phdr);

    const u64 interp_off = cur;
    cur += sizeof INTERP;

    cur = align_up(cur, 8);
    const u64 dynsym_off = cur;
    cur += (u64)nsym * sizeof(Elf64_Sym);

    const u64 dynstr_off = cur;
    cur += dynstr_len;

    cur = align_up(cur, 8);
    const u64 hash_off = cur;
    const u64 hash_size = (2 + (u64)nbucket + (u64)nsym) * sizeof(u32);
    cur += hash_size;

    cur = align_up(cur, 8);
    const u64 rela_off = cur;
    const u64 rela_size = (u64)nimp * sizeof(Elf64_Rela);
    cur += rela_size;

    cur = align_up(cur, 16);
    const u64 stub_off = cur;
    cur += DYN_START_STUB_SIZE;

    const u64 plt_off = cur;
    cur += (u64)nplt * PLT_ENTRY_SIZE;

    cur = align_up(cur, 16);
    const u64 text_off = cur;
    cur += image->text_size;
    const u64 ro_filesz = cur;

    const u64 rw_off = align_up(cur, PAGE);
    const u64 dynamic_off = rw_off;
    const u64 got_off = dynamic_off + (u64)ndyn * sizeof(Elf64_Dyn);
    const u64 got_vaddr = LOAD_BASE + got_off;
    /* GOT_RESERVED slots + one per import */
    const u64 got_size = ((u64)GOT_RESERVED + (u64)nimp) * 8;
    const u64 rw_filesz = (got_off + got_size) - rw_off;
    const u64 image_size = got_off + got_size;

    if (image_size > sizeof img || dynstr_len > sizeof dynstr || nimp > 62) {
        report_error("elf-exe: dynamic image too large\n");
        return;
    }
    memset(img, 0, image_size);

    Elf64_Ehdr *eh = (Elf64_Ehdr *)(img + ehdr_off);
    eh->e_ident[EI_MAG0] = ELFMAG0;
    eh->e_ident[EI_MAG1] = ELFMAG1;
    eh->e_ident[EI_MAG2] = ELFMAG2;
    eh->e_ident[EI_MAG3] = ELFMAG3;
    eh->e_ident[EI_CLASS] = ELFCLASS64;
    eh->e_ident[EI_DATA] = ELFDATA2LSB;
    eh->e_ident[EI_VERSION] = EV_CURRENT;
    eh->e_type = ET_EXEC;
    eh->e_machine = EM_X86_64;
    eh->e_version = EV_CURRENT;
    eh->e_entry = LOAD_BASE + stub_off;
    eh->e_phoff = phdr_off;
    eh->e_ehsize = sizeof(Elf64_Ehdr);
    eh->e_phentsize = sizeof(Elf64_Phdr);
    eh->e_phnum = (u16)nphdr;

    Elf64_Phdr *ph = (Elf64_Phdr *)(img + phdr_off);
    ph[0].p_type = PT_PHDR;
    ph[0].p_flags = PF_R;
    ph[0].p_offset = phdr_off;
    ph[0].p_vaddr = LOAD_BASE + phdr_off;
    ph[0].p_paddr = LOAD_BASE + phdr_off;
    ph[0].p_filesz = (u64)nphdr * sizeof(Elf64_Phdr);
    ph[0].p_memsz = (u64)nphdr * sizeof(Elf64_Phdr);
    ph[0].p_align = 8;

    ph[1].p_type = PT_INTERP;
    ph[1].p_flags = PF_R;
    ph[1].p_offset = interp_off;
    ph[1].p_vaddr = LOAD_BASE + interp_off;
    ph[1].p_paddr = LOAD_BASE + interp_off;
    ph[1].p_filesz = sizeof INTERP;
    ph[1].p_memsz = sizeof INTERP;
    ph[1].p_align = 1;

    ph[2].p_type = PT_LOAD;
    ph[2].p_flags = PF_R | PF_X;
    ph[2].p_offset = 0;
    ph[2].p_vaddr = LOAD_BASE;
    ph[2].p_paddr = LOAD_BASE;
    ph[2].p_filesz = ro_filesz;
    ph[2].p_memsz = ro_filesz;
    ph[2].p_align = PAGE;

    ph[3].p_type = PT_LOAD;
    ph[3].p_flags = PF_R | PF_W;
    ph[3].p_offset = rw_off;
    ph[3].p_vaddr = LOAD_BASE + rw_off;
    ph[3].p_paddr = LOAD_BASE + rw_off;
    ph[3].p_filesz = rw_filesz;
    ph[3].p_memsz = rw_filesz;
    ph[3].p_align = PAGE;

    ph[4].p_type = PT_DYNAMIC;
    ph[4].p_flags = PF_R | PF_W;
    ph[4].p_offset = dynamic_off;
    ph[4].p_vaddr = LOAD_BASE + dynamic_off;
    ph[4].p_paddr = LOAD_BASE + dynamic_off;
    ph[4].p_filesz = (u64)ndyn * sizeof(Elf64_Dyn);
    ph[4].p_memsz = (u64)ndyn * sizeof(Elf64_Dyn);
    ph[4].p_align = 8;

    memcpy(img + interp_off, INTERP, sizeof INTERP);

    Elf64_Sym *syms = (Elf64_Sym *)(img + dynsym_off);
    for (u32 i = 0; i < nimp; i++) {
        syms[i + 1].st_name = sym_name_off[i];
        syms[i + 1].st_info = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC);
        syms[i + 1].st_shndx = SHN_UNDEF;
    }

    memcpy(img + dynstr_off, dynstr, dynstr_len);

    u32 *hash = (u32 *)(img + hash_off);
    hash[0] = nbucket;
    hash[1] = nsym;
    u32 *bucket = hash + 2;
    u32 *chain = bucket + nbucket;
    for (u32 i = 1; i < nsym; i++) {
        u32 h = elf_hash(import_name(image, i - 1)) % nbucket;
        chain[i] = bucket[h];
        bucket[h] = i;
    }

    Elf64_Rela *rela = (Elf64_Rela *)(img + rela_off);
    for (u32 i = 0; i < nimp; i++) {
        rela[i].r_offset = got_vaddr + ((u64)GOT_RESERVED + i) * 8;
        rela[i].r_info = ELF64_R_INFO(i + 1, R_X86_64_JUMP_SLOT);
        rela[i].r_addend = 0;
    }

    Elf64_Dyn *dyn = (Elf64_Dyn *)(img + dynamic_off);
    int d = 0;
    dyn[d].d_tag = DT_NEEDED;      dyn[d++].d_un.d_val = lib_name_off;
    dyn[d].d_tag = DT_HASH;        dyn[d++].d_un.d_ptr = LOAD_BASE + hash_off;
    dyn[d].d_tag = DT_STRTAB;      dyn[d++].d_un.d_ptr = LOAD_BASE + dynstr_off;
    dyn[d].d_tag = DT_SYMTAB;      dyn[d++].d_un.d_ptr = LOAD_BASE + dynsym_off;
    dyn[d].d_tag = DT_STRSZ;       dyn[d++].d_un.d_val = dynstr_len;
    dyn[d].d_tag = DT_SYMENT;      dyn[d++].d_un.d_val = sizeof(Elf64_Sym);
    dyn[d].d_tag = DT_PLTGOT;      dyn[d++].d_un.d_ptr = got_vaddr;
    dyn[d].d_tag = DT_PLTRELSZ;    dyn[d++].d_un.d_val = rela_size;
    dyn[d].d_tag = DT_PLTREL;      dyn[d++].d_un.d_val = DT_RELA;
    dyn[d].d_tag = DT_JMPREL;      dyn[d++].d_un.d_ptr = LOAD_BASE + rela_off;
    dyn[d].d_tag = DT_NULL;        dyn[d++].d_un.d_val = 0;

    const u64 text_vaddr = LOAD_BASE + text_off;
    u64 main_vaddr = text_vaddr + image->entry;
    u64 call_main_next = LOAD_BASE + stub_off + 5;
    int32_t call_main_rel = (int32_t)((i64)main_vaddr - (i64)call_main_next);
    /* exit is PLT[exit_idx+1] because PLT[0] is the resolver */
    u64 exit_plt_vaddr = LOAD_BASE + plt_off + (u64)(exit_idx + 1) * PLT_ENTRY_SIZE;
    u64 call_exit_next = LOAD_BASE + stub_off + DYN_START_STUB_SIZE;
    int32_t call_exit_rel = (int32_t)((i64)exit_plt_vaddr - (i64)call_exit_next);
    build_dyn_start_stub(img + stub_off, call_main_rel, call_exit_rel);

    /* GOT[0] = .dynamic vaddr (ld.so also writes it, but we set it for clarity) */
    u64 *got = (u64 *)(img + got_off);
    got[0] = LOAD_BASE + dynamic_off;
    /* GOT[1], GOT[2] = 0; ld.so fills them with link-map and resolver */

    /* PLT[0]: resolver stub
       push QWORD PTR [rip+GOT[1]]; jmp QWORD PTR [rip+GOT[2]]; nop*4 */
    {
        u8 *p = img + plt_off;
        u64 got1_vaddr = got_vaddr + 8;
        u64 got2_vaddr = got_vaddr + 16;
        u64 push_next = LOAD_BASE + plt_off + 6;
        u64 jmp_next  = LOAD_BASE + plt_off + 12;
        int32_t r1 = (int32_t)((i64)got1_vaddr - (i64)push_next);
        int32_t r2 = (int32_t)((i64)got2_vaddr - (i64)jmp_next);
        p[0] = 0xff; p[1] = 0x35; memcpy(p + 2, &r1, 4);
        p[6] = 0xff; p[7] = 0x25; memcpy(p + 8, &r2, 4);
        p[12] = 0x0f; p[13] = 0x1f; p[14] = 0x40; p[15] = 0x00;
    }

    /* PLT[n] for each import (n = i+1):
       jmp [rip+GOT[GOT_RESERVED+i]]; push i; jmp PLT[0] */
    for (u32 i = 0; i < nimp; i++) {
        u8 *p = img + plt_off + (u64)(i + 1) * PLT_ENTRY_SIZE;
        u64 got_entry_vaddr = got_vaddr + ((u64)GOT_RESERVED + i) * 8;
        u64 jmp_next  = LOAD_BASE + plt_off + (u64)(i + 1) * PLT_ENTRY_SIZE + 6;
        u64 push_next = jmp_next + 5;
        u64 plt0_vaddr = LOAD_BASE + plt_off;
        int32_t jmp_got_rel  = (int32_t)((i64)got_entry_vaddr - (i64)jmp_next);
        int32_t jmp_plt0_rel = (int32_t)((i64)plt0_vaddr - (i64)(push_next + 5));
        p[0] = 0xff; p[1] = 0x25; memcpy(p + 2, &jmp_got_rel, 4);
        p[6] = 0x68; memcpy(p + 7, &i, 4);
        p[11] = 0xe9; memcpy(p + 12, &jmp_plt0_rel, 4);

        /* GOT[GOT_RESERVED+i] initially points to the push instruction (PLT[n]+6)
           so the first call falls through to the lazy resolver */
        got[GOT_RESERVED + i] = jmp_next;
    }

    memcpy(img + text_off, image->text, image->text_size);
    patch_extcalls(image, text_off, plt_off);

    finish(out, image_size);
}

void emit_init(void) {}

void emit_output(FILE *out) {
    bin_image image;
    bin_emit(&image);
    if (image.imports_count > 0)
        write_dynamic_elf(out, &image);
    else
        write_static_elf(out, &image);
}

const char *text_section_header = "";
const char *string_section_header = "";
const char *output_ext = "";
