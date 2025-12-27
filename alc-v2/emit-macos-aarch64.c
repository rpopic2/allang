#include <stdio.h>

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


void emit_init(void) {
    buf_init(text_buf, INIT_BUFSIZ);
    buf_puts(text_buf, STR_FROM_INSTR(".section	__TEXT,__text,regular,pure_instructions"));

    buf_init(cstr_buf, INIT_BUFSIZ);
    buf_puts(cstr_buf, STR_FROM_INSTR("\n\n\t.section	__TEXT,__cstring,cstring_literals"));
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

void emit_mov(register_dst reg_dst, int regidx, int value) {
    if (reg_dst == SCRATCH)
        regidx += 8;
    else if (reg_dst == NREG)
        regidx += 19;
    buf_snprintf(fn_buf, INSTR("mov w%d, #%d"), regidx, value);
}

void emit_string_lit(register_dst reg_dst, int regidx, const str *s) {
    if (reg_dst == SCRATCH)
        regidx += 8;

    char *buffer = malloc(SPRINTF_BUFSIZ);
    int num_printed = snprintf(buffer, SPRINTF_BUFSIZ, "l_.str.%d", string_lit_counts++);
    if (num_printed >= SPRINTF_BUFSIZ) {
        fputs("buffer overflow in snprintf\n", stderr);
        exit(EXIT_FAILURE);
    }
    buf_snprintf(fn_buf, INSTR("adrp x%d, %s@PAGE"), regidx, buffer);
    buf_snprintf(fn_buf, INSTR("add x%d, x%d, %s@PAGEOFF"), regidx, regidx, buffer);

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

    int stack_size = regs_to_save * sizeof (uint64_t); // plus context stack size
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
    buf_puts(fn_buf, &STR_FROM("\tbl _"));
    buf_puts(fn_buf, s);
    buf_putc(fn_buf, '\n');
}

void emit_mainfn(void) {
    buf_puts(text_buf, &STR_FROM("\t.globl _main\n\t.p2align 2\n_main:\n"));
}

void emit_ret(void) {
    buf_puts(fn_buf, STR_FROM_INSTR("ret"));
}

