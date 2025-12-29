#include <stdio.h>
#include <inttypes.h>

#include "buffer.h"
#include "emit.h"
#include "str.h"
#include "err.h"

#define INIT_BUFSIZ 0x400
#define CALLEE_START 19

#define DECL_PTR(T, X) T _##X; T *X = &_##X

#define INSTR(s) "\t"s"\n"
#define STR_FROM_INSTR(s) &STR_FROM(INSTR(s))

DECL_PTR(static buf, text_buf);
DECL_PTR(static buf, cstr_buf);

DECL_PTR(static buf, prologue_buf);
DECL_PTR(static buf, fn_buf);

char *cstr_begin = NULL;
unsigned string_lit_counts = 0;

extern const char *addrgen_adrp;
extern const char *addrgen_add;
extern const char *fn_prefix;
extern const char *mainfn_annotation;
extern const char *local_string_prefix;


void emit_init(void) {
    buf_init(text_buf, INIT_BUFSIZ);
    buf_puts(text_buf, &STR_FROM(text_section_header));

    buf_init(cstr_buf, INIT_BUFSIZ);
    buf_puts(cstr_buf, &STR_FROM(string_section_header));
    cstr_begin = cstr_buf->cur;

    buf_init(prologue_buf, INIT_BUFSIZ);
    buf_init(fn_buf, INIT_BUFSIZ);
}

void emit(FILE *out) {
    buf_fwrite(text_buf, out);
    buf_fwrite(prologue_buf, out);
    buf_fwrite(fn_buf, out);
    if (cstr_begin < cstr_buf->cur) {
        buf_fwrite(cstr_buf, out);
    }
}

bool emit_need_escaping(void) {
    return false;
}

int get_regoff(entry e) {
    if (e.type == SCRATCH)
        e.offset += 8;
    else if (e.type == NREG)
        e.offset += 19;
    return e.offset;
}

void emit_mov(register_dst reg_dst, int regidx, i64 value) {
    regidx = get_regoff((entry){reg_dst, regidx});
    buf_snprintf(fn_buf, INSTR("mov w%d, #"PRId64), regidx, value);
}

void emit_mov_reg(register_dst reg_dst, int regidx, register_dst reg_src, int regidx_src) {
    int dst = get_regoff((entry){reg_dst, regidx});
    int src = get_regoff((entry){reg_src, regidx_src});

    buf_snprintf(fn_buf, INSTR("mov w%d, w%d"), dst, src);
}

void emit_add(entry dst, entry lhs, i64 rhs) {
    buf_snprintf(fn_buf, INSTR("add w%d, w%d, #"PRId64),
            get_regoff(dst), get_regoff(lhs), rhs);
}

void emit_add_reg(entry dst, entry lhs, entry rhs) {
    buf_snprintf(fn_buf, INSTR("add w%d, w%d, w%d"),
            get_regoff(dst), get_regoff(lhs), get_regoff(rhs));
}

void emit_sub(entry dst, entry lhs, i64 rhs) {
    buf_snprintf(fn_buf, INSTR("sub w%d, w%d, #"PRId64),
            get_regoff(dst), get_regoff(lhs), rhs);
}
void emit_sub_reg(entry dst, entry lhs, entry rhs) {
    buf_snprintf(fn_buf, INSTR("sub w%d, w%d, w%d"),
            get_regoff(dst), get_regoff(lhs), get_regoff(rhs));
}

void emit_string_lit(register_dst reg_dst, int regidx, const str *s) {
    if (reg_dst == SCRATCH)
        regidx += 8;

    char *buffer = malloc(SPRINTF_BUFSIZ);
    int num_printed = snprintf(buffer, SPRINTF_BUFSIZ, local_string_prefix, string_lit_counts++);
    if (num_printed >= SPRINTF_BUFSIZ) {
        fputs("buffer overflow in snprintf\n", stderr);
        exit(EXIT_FAILURE);
    }
    buf_snprintf(fn_buf, addrgen_adrp, regidx, buffer);
    buf_snprintf(fn_buf, addrgen_add, regidx, regidx, buffer);

    buf_snprintf(cstr_buf, "%s:\n", buffer);
    buf_puts(cstr_buf, &STR_FROM("\t.asciz "));
    buf_puts(cstr_buf, s);
    buf_putc(cstr_buf, '\n');

    free(buffer);
}

void emit_fn_prologue_epilogue(const parser_context *context) {
    prologue_buf->cur = prologue_buf->start;
    if (!context->calls_fn && context->nreg_count == 0) {
        return;
    }

    int regs_to_save = context->nreg_count;
    if (regs_to_save + CALLEE_START >= 28) {
        compile_err("used up all callee-saved registers");
        return;
    }

    int stack_size = regs_to_save * (signed)sizeof (uint64_t); // plus context stack size
    if (context->calls_fn) {
        stack_size += 16;
    }
    stack_size = (stack_size + 15) & ~15; // align up to 16 bytes boundary

    int cur_stackoff = 0;
    int frame_stackoff = 0;// + stack variable size;
    if (context->calls_fn) {
        if (frame_stackoff == 0) {
            buf_snprintf(prologue_buf, INSTR("stp x29, x30, [sp, #-%d]!"), stack_size);
        } else {
            buf_snprintf(prologue_buf, INSTR("sub sp, sp, #%d"), stack_size);
            buf_snprintf(prologue_buf, INSTR("stp x29, x30, [sp, #%d]"), frame_stackoff);
        }
        cur_stackoff += 16;

    }
    int tmp = regs_to_save;

    if (tmp % 2 == 1) {
        tmp -= 1;
        buf_snprintf(prologue_buf, INSTR("str x%d, [sp, #%d]"),
                CALLEE_START + tmp, cur_stackoff);
        buf_snprintf(fn_buf, INSTR("ldr x%d, [sp, #%d]"),
                CALLEE_START + tmp, cur_stackoff);
        cur_stackoff += 16;
    }
    while (tmp > 0) {
        buf_snprintf(prologue_buf, INSTR("stp x%d, x%d, [sp, #%d]"),
                CALLEE_START + tmp - 1, CALLEE_START + tmp - 2, cur_stackoff);
        buf_snprintf(fn_buf, INSTR("ldp x%d, x%d, [sp, #%d]"),
                CALLEE_START + tmp - 1, CALLEE_START + tmp - 2, cur_stackoff);
        tmp -= 2;
        cur_stackoff += 16;
    }

    if (context->calls_fn) {
        if (frame_stackoff == 0) {
            buf_puts(prologue_buf, STR_FROM_INSTR("mov x29, sp"));
            buf_snprintf(fn_buf, INSTR("stp x29, x30, [sp], #%d"), stack_size);
        } else {
            buf_snprintf(prologue_buf, INSTR("add x29, sp, #%d"), frame_stackoff);
            buf_snprintf(fn_buf, INSTR("ldp x29, x30, [sp, #%d]"), frame_stackoff);
            buf_snprintf(fn_buf, INSTR("add sp, sp, #%d"), stack_size);
        }
    }
}

void emit_fn_call(const str *s) {
    buf_puts(fn_buf, &STR_FROM("\tbl "));
    buf_puts(fn_buf, &STR_FROM(fn_prefix));
    buf_puts(fn_buf, s);
    buf_putc(fn_buf, '\n');
}

void emit_mainfn(void) {
    buf_puts(text_buf, STR_FROM_INSTR(".globl _main"));
    buf_puts(text_buf, STR_FROM_INSTR(".p2align 2"));
    buf_puts(text_buf, &STR_FROM(mainfn_annotation));
    buf_puts(text_buf, &STR_FROM(fn_prefix));
    buf_puts(text_buf, &STR_FROM("main:"));
}

void emit_ret(void) {
    buf_puts(fn_buf, STR_FROM_INSTR("ret"));
}

