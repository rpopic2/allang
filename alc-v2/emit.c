#include <stdio.h>

#include "buffer.h"
#include "emit.h"
#include "str.h"

#define INIT_BUFSIZ 0x400

#define DECL_PTR(T, X) T _##X; T *X = &_##X

#define INSTR(s) "\t"s"\n"
#define STR_FROM_INSTR(s) &STR_FROM(INSTR(s))

DECL_PTR(static buf, text_buf);
DECL_PTR(static buf, cstr_buf);
char *cstr_begin;


void emit_init(void) {
    buf_init(text_buf, INIT_BUFSIZ);
    buf_puts(text_buf, STR_FROM_INSTR(".section	__TEXT,__text,regular,pure_instructions"));

    buf_init(cstr_buf, INIT_BUFSIZ);
    buf_puts(cstr_buf, STR_FROM_INSTR("\n\n\t.section	__TEXT,__cstring,cstring_literals"));
    cstr_begin = cstr_buf->cur;
}

void emit(FILE *out) {
    buf_fwrite(text_buf, out);
    if (cstr_begin < cstr_buf->cur) {
        buf_fwrite(cstr_buf, out);
    }
}

void emit_mov_retreg(int regidx, int value) {
    buf_snprintf(text_buf, INSTR("mov w%d, #%d"), regidx, value);
}

void emit_mov_scratch(int regidx, int value) {
    buf_snprintf(text_buf, INSTR("mov w%d, #%d"), regidx + 8, value);
}

void emit_mov_param(int regidx, int value) {
    buf_snprintf(text_buf, INSTR("mov w%d, #%d"), regidx, value);
}

void emit_string_lit(int regidx, const str *s) {
    const char *id = "l_.str";
    buf_snprintf(text_buf, INSTR("adrp x%d, %s@PAGE"), regidx, id);
    buf_snprintf(text_buf, INSTR("add x%d, x%d, %s@PAGEOFF"), regidx, regidx, id);

    buf_snprintf(cstr_buf, "%s:\n", id);
    buf_puts(cstr_buf, &STR_FROM("\t.asciz "));
    buf_puts(cstr_buf, s);
    buf_putc(cstr_buf, '\n');
}

void emit_fn_call(const str *s) {
    buf_puts(text_buf, &STR_FROM("\tbl "));
    buf_puts(text_buf, s);
    buf_putc(text_buf, '\n');
}

void emit_mainfn(void) {
    buf_puts(text_buf, &STR_FROM(".globl _main\n.p2align 2\n_main:\n"));
}

void emit_ret(void) {
    buf_puts(text_buf, STR_FROM_INSTR("ret"));
}

