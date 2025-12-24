#include <stdio.h>

#include "buffer.h"
#include "emit.h"

#define INIT_BUFSIZ 0x400

static buf *text_buf = &(buf){.start = 0, .end = 0, .cur = 0};

void emit_init(void) {
    buf_init(text_buf, INIT_BUFSIZ);
}
void emit(FILE *out) {
    buf_fwrite(text_buf, out);
}

bool emit_need_escaping(void) {
    return false;
}

void emit_mov(register_dst reg_dst, int regidx, int value);
void emit_string_lit(register_dst reg_dst, int regidx, const str *s);

void emit_fn_prologue(void);
void emit_fn_epilogue(void);
void emit_fn_call(const str *s);
void emit_mainfn(void);
void emit_ret(void);

