#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "emit.h"
#include "typesys.h"
#include "err.h"

/* Direct ELF backend: instead of emitting textual assembly for an external
   assembler and linker, this backend writes a complete, statically linked
   x86_64 Linux ELF executable straight to the output file.

   Machine-code generation for the parsed program is not implemented yet, so
   every instruction emitter is a no-op and the produced executable always runs
   a fixed stub that performs exit(0). */

const size_t default_register_size = 8;

bool emit_need_escaping(void) {
    return false;
}

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

#define ELF_LOAD_BASE 0x400000UL
#define ELF_PT_LOAD 1u
#define ELF_PF_X 1u
#define ELF_PF_R 4u
#define ELF_ET_EXEC 2u
#define ELF_EM_X86_64 62u

/* _start:
       xor edi, edi    ; exit status 0
       mov eax, 60     ; __NR_exit
       syscall                                  */
static const u8 code_stub[] = {
    0x31, 0xff,
    0xb8, 0x3c, 0x00, 0x00, 0x00,
    0x0f, 0x05,
};

void emit_init(void) {
}

void emit_fn_begin(emit_context_t *context) {
    (void)context;
}

void emit_fn_end(emit_context_t *context) {
    (void)context;
}

void emit_output(FILE *out) {
    const u64 headers_size = sizeof(elf64_ehdr) + sizeof(elf64_phdr);
    const u64 entry = ELF_LOAD_BASE + headers_size;
    const u64 image_size = headers_size + sizeof code_stub;

    const elf64_ehdr ehdr = {
        .e_ident = {0x7f, 'E', 'L', 'F', 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        .e_type = ELF_ET_EXEC,
        .e_machine = ELF_EM_X86_64,
        .e_version = 1,
        .e_entry = entry,
        .e_phoff = sizeof(elf64_ehdr),
        .e_shoff = 0,
        .e_flags = 0,
        .e_ehsize = (u16)sizeof(elf64_ehdr),
        .e_phentsize = (u16)sizeof(elf64_phdr),
        .e_phnum = 1,
        .e_shentsize = 0,
        .e_shnum = 0,
        .e_shstrndx = 0,
    };

    const elf64_phdr phdr = {
        .p_type = ELF_PT_LOAD,
        .p_flags = ELF_PF_R | ELF_PF_X,
        .p_offset = 0,
        .p_vaddr = ELF_LOAD_BASE,
        .p_paddr = ELF_LOAD_BASE,
        .p_filesz = image_size,
        .p_memsz = image_size,
        .p_align = 0x1000,
    };

    fwrite(&ehdr, sizeof ehdr, 1, out);
    fwrite(&phdr, sizeof phdr, 1, out);
    fwrite(code_stub, sizeof code_stub, 1, out);
}

void emit_make_array(reg_t dst, type_t *type, u32 len, dyn_regable *args) {
    (void)dst;
    (void)type;
    (void)len;
    (void)args;
}

void emit_store_array(reg_t dst, i64 offset, type_t *type, u32 len, dyn_regable *args) {
    (void)dst;
    (void)offset;
    (void)type;
    (void)len;
    (void)args;
}

void emit_array_access(reg_t dst, reg_t src, reg_t offset, load_store_t is_store) {
    (void)dst;
    (void)src;
    (void)offset;
    (void)is_store;
}

void emit_elem_addr(reg_t dst, reg_t object, reg_t index) {
    (void)dst;
    (void)object;
    (void)index;
}

bool emit_eightbyte_struct(reg_t dst, const dtype_t *dtype, const dyn_agg_member *args, int *index, size_t *size, size_t limit) {
    (void)dst;
    (void)dtype;
    (void)args;
    (void)index;
    (void)size;
    (void)limit;
    return false;
}

void emit_store_eightbytes(reg_t base, i64 offset, reg_t lo, bool lo_written, reg_t hi, bool hi_written, bool has_hi) {
    (void)base;
    (void)offset;
    (void)lo;
    (void)lo_written;
    (void)hi;
    (void)hi_written;
    (void)has_hi;
}

void emit_store_packed(reg_t base, i64 offset, reg_t src, size_t nbytes) {
    (void)base;
    (void)offset;
    (void)src;
    (void)nbytes;
}

void emit_zerofill(reg_t dst, i64 offset, const dtype_t *type) {
    (void)dst;
    (void)offset;
    (void)type;
}

void emit_mov(reg_t dst, i64 value) {
    (void)dst;
    (void)value;
}

void emit_mov_reg(reg_t dst, reg_t src) {
    (void)dst;
    (void)src;
}

void emit_add(reg_t dst, reg_t lhs, i64 rhs) {
    (void)dst;
    (void)lhs;
    (void)rhs;
}

void emit_add_reg(reg_t dst, reg_t lhs, reg_t rhs) {
    (void)dst;
    (void)lhs;
    (void)rhs;
}

void emit_sub(reg_t dst, reg_t lhs, i64 rhs) {
    (void)dst;
    (void)lhs;
    (void)rhs;
}

void emit_sub_reg(reg_t dst, reg_t lhs, reg_t rhs) {
    (void)dst;
    (void)lhs;
    (void)rhs;
}

void emit_cmp(reg_t lhs, i64 rhs) {
    (void)lhs;
    (void)rhs;
}

void emit_cmp_reg(reg_t lhs, reg_t rhs, cond_t cond) {
    (void)lhs;
    (void)rhs;
    (void)cond;
}

void emit_string_lit(reg_t dst, const str *s) {
    (void)dst;
    (void)s;
}

void emit_lsl(reg_t dst, reg_t lhs, i64 rhs) {
    (void)dst;
    (void)lhs;
    (void)rhs;
}

void emit_zero_out(reg_t dst) {
    (void)dst;
}

void emit_cond_set(reg_t dst, cond_t cond) {
    (void)dst;
    (void)cond;
}

void emit_str_reg(reg_t dst, reg_t src, int offset) {
    (void)dst;
    (void)src;
    (void)offset;
}

void emit_str_imm(reg_t dst, i64 value, int offset) {
    (void)dst;
    (void)value;
    (void)offset;
}

void emit_ldr(reg_t dst, reg_t src, int offset) {
    (void)dst;
    (void)src;
    (void)offset;
}

void emit_str_regoff(reg_t dst, reg_t src, reg_t offset) {
    (void)dst;
    (void)src;
    (void)offset;
}

void emit_str_imm_regoff(reg_t dst, i64 value, reg_t offset) {
    (void)dst;
    (void)value;
    (void)offset;
}

void emit_ldr_reg(reg_t dst, reg_t src, reg_t offset) {
    (void)dst;
    (void)src;
    (void)offset;
}

void emit_branch(str fn_name, str label, int index) {
    (void)fn_name;
    (void)label;
    (void)index;
}

bool emit_branch_cond(cond_t condition, str fn_name, str label, int index) {
    (void)condition;
    (void)fn_name;
    (void)label;
    (void)index;
    return true;
}

void emit_label(str fn_name, str label, int index) {
    (void)fn_name;
    (void)label;
    (void)index;
}

void emit_fn_prologue_epilogue(const parser_context *context) {
    (void)context;
}

void emit_fn_call(const str *s) {
    (void)s;
}

void emit_fn(str fn_name) {
    (void)fn_name;
}

void emit_ret(void) {
}
