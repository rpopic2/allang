#pragma once

#include "str.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct {
    char *start;
    char *end;
    char *cur;
} buf;

typedef struct {
    void *start;
    void *end;
    void *cur;
} fixed_arena;

inline size_t buf_cap(const buf *buffer) {
    if (buffer->end - buffer->start < 0)
        abort();
    return (size_t)(buffer->end - buffer->start);
}

inline size_t buf_len(const buf *buffer) {
    if (buffer->cur - buffer->start < 0)
        abort();
    return (size_t)(buffer->cur - buffer->start);
}

inline void buf_grow(buf *buffer, size_t adding_size) {
    while (buffer->cur + adding_size >= buffer->end) {
        size_t len = buf_cap(buffer);
        ptrdiff_t pos = buffer->cur - buffer->start;
        size_t new_len = len * 2;
        buffer->start = realloc(buffer->start, new_len);
        buffer->cur = buffer->start + pos;
        buffer->end = buffer->start + new_len;
    }
}

inline void buf_init(buf *buffer, size_t size) {
    buffer->start = malloc(size);
    buffer->cur = buffer->start;
    buffer->end = buffer->start + size;
}

inline void buf_puts(buf *buffer, const str *s) {
    size_t len = str_len(s);
    buf_grow(buffer, len);

    memcpy(buffer->cur, s->data, len);
    buffer->cur += len;
}

inline void buf_putc(buf *buffer, char c) {
    buf_grow(buffer, 1);

    *buffer->cur++ = c;
}

#define SPRINTF_BUFSIZ 0x400
_Thread_local static char sprintf_buf[SPRINTF_BUFSIZ];

static void buf_snprintf(buf *buffer, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int num_printed = vsnprintf(sprintf_buf, SPRINTF_BUFSIZ, format, args);
    va_end(args);

    if (num_printed >= SPRINTF_BUFSIZ) {
        fputs("buffer overflow in snprintf\n", stderr);
        exit(EXIT_FAILURE);
    }

    buf_grow(buffer, (size_t)num_printed);

    strcpy(buffer->cur, sprintf_buf);
    buffer->cur += num_printed;
}

void buf_fwrite(const buf *buffer, FILE *out) {
    fwrite(buffer->start, sizeof (char), buf_len(buffer), out);
}
