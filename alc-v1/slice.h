#pragma once

#include "str.h"
#include "typedefs.h"
#include <stdbool.h>
#include <string.h>


typedef struct {
    char *data;
    char *end;
} str_iter;

#define ITER_EOF 0
int iter_next(str_iter *self) {
    if (self->data < self->end) {
        ++(self->data);
        return *self->data;
    }
    return ITER_EOF;
}
int iter_prev(str_iter *self) {
    return *(--(self->data));
}

static inline str_iter into_iter(str s) {
    return (str_iter) { .data = s.data - 1, .end = s.data + s.len };
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
