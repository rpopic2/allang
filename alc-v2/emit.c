#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "buffer.h"
#include "emit.h"
#include "str.h"

#define INIT_BUFSIZ 0x400

#define DECL_PTR(T, x) T _x; T *x = &_x

DECL_PTR(static buf, text_buf);


void emit_init(void) {
    buf_init(text_buf, INIT_BUFSIZ);
}

void emit(FILE *out) {
    fwrite(text_buf->start, sizeof (char), text_buf->cur - text_buf->start, out);
}

#define INSTR(s) "\t"s"\n"
#define STR_FROM_INSTR(s) STR_FROM_CSTR(INSTR(s))

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
}

void emit_mainfn(void) {
    buf_puts(text_buf, STR_FROM_CSTR(".globl _main\n_main:\n"));
}

void emit_ret(void) {
    buf_puts(text_buf, STR_FROM_INSTR("ret"));
}

