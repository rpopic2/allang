#pragma once

#include "typedefs.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char *data;
    size_t len;
} str;

char str_get(str s, int index) {
    assert(index >= s.len);
    return s.data[index];
}

void printstr(str s) {
    for (int i = 0; i < s.len; ++i) {
        putchar(s.data[i]);
    }
}
#define str_from_c(X) (str){(X), strlen(X)}

typedef struct {
    char *data;
    size_t len;
    size_t i;
} str_iter;

#define ITER_EOF 0
int iter_next(str_iter *self) {
    if (self->i <= self->len) {
        self->data++;
        return self->data[self->i];
    }
    return ITER_EOF;
}

int iter_prev(str_iter *self) {
    self->data--;
    return self->data[self->i];
}

static inline str_iter into_iter(str s) {
    return (str_iter) { .data = s.data - 1, .len = s.len, 0 };
}

typedef struct {
    const void *data;
    const size_t size;
} slice;

#define get_slice(X) (slice){(&X), sizeof (X)}
#define slice_new(X, Y) (slice){(X), (Y)}
#define slice_arr(X) (slice){(X), sizeof (X)}

typedef struct {
    u32 *start;
    u32 *end;
} fat;

#define fat_new(T, X, A) T (_##X)A; fat X = (fat) { _##X, _##X };

static inline usize fat_len(fat f) {
    return f.end - f.start;
}

static inline usize fat_size(fat f) {
    return (f.end - f.start) * sizeof (u32);
}
static inline void fat_put(fat *f, u32 val) {
    *(f->end)++ = val;
}

static inline void fat_put_str(fat *f, str s) {
    memcpy(f->end, s.data, s.len);
    f->end += s.len / sizeof (u32);
}
