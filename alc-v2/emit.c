#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "emit.h"
#include "str.h"

typedef struct {
    char *start;
    char *end;
    char *cur;
} buf;

#define BUF_LEN(buf) (buf.end - buf.start);

static buf buffer;

#define INIT_BUFSIZ 0x400

void buf_grow(size_t adding_size) {
    while (buffer.cur + adding_size >= buffer.end) {
        printf("realloc!\n");
        size_t len = BUF_LEN(buffer);
        size_t pos = buffer.cur - buffer.start;
        size_t new_len = len * 2;
        buffer.start = realloc(buffer.start, new_len);
        buffer.cur = buffer.start + pos;
        buffer.end = buffer.start + new_len;
    }
}

void buf_puts(str s) {
    size_t len = str_len(&s);
    buf_grow(len);

    memcpy(buffer.cur, s.data, len);
    buffer.cur += len;
}

#define SPRINTF_BUFSIZ 0x400
static char sprintf_buf[SPRINTF_BUFSIZ];

void buf_snprintf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int num_printed = vsnprintf(sprintf_buf, SPRINTF_BUFSIZ, format, args);
    va_end(args);

    if (num_printed >= SPRINTF_BUFSIZ) {
        fputs("buffer overflow in snprintf\n", stderr);
        exit(EXIT_FAILURE);
    }

    buf_grow(num_printed);

    strcpy(buffer.cur, sprintf_buf);
    buffer.cur += num_printed;
}


void emit_init(void) {
    buffer.start = malloc(INIT_BUFSIZ);
    buffer.cur = buffer.start;
    buffer.end = buffer.start + INIT_BUFSIZ;
}

void emit(FILE *out) {
    fwrite(buffer.start, sizeof (char), buffer.cur - buffer.start, out);
}

#define INSTR(s) "\t"s"\n"
#define STR_FROM_INSTR(s) STR_FROM_CSTR(INSTR(s))

void emit_mov_retreg(int regidx, int value) {
    buf_snprintf(INSTR("mov w%d, #%d"), regidx, value);
}

void emit_mov_scratch(int regidx, int value) {
    buf_snprintf(INSTR("mov w%d, #%d"), regidx + 8, value);
}

void emit_mov_param(int regidx, int value) {
    buf_snprintf(INSTR("mov w%d, #%d"), regidx, value);
}

void emit_mainfn(void) {
    buf_puts(STR_FROM_CSTR(".globl _main\n_main:\n"));
}

void emit_ret(void) {
    buf_puts(STR_FROM_INSTR("ret"));
}

