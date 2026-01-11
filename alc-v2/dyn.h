// dynamic arrays
#pragma once

#include "types.h"

#define DYN_GENERIC(T) \
typedef struct { \
    const struct_t **begin; \
    const struct_t **cur; \
    const struct_t **end; \
} dyn_T; \
 \
void dyn_T_realloc(dyn_T *self) { \
    if (self->end - self->begin < 0) \
        unreachable; \
    usize len = (usize)(self->end - self->begin); \
    if (len == 0) { \
        len = 4; \
        size_t size = len * sizeof (struct_t *); \
        self->begin = malloc(size); \
        self->end = self->begin + size; \
    } else { \
        len *= 2; \
        size_t size = len * sizeof (struct_t *); \
        self->begin = realloc(self->begin, len * sizeof (struct_t *)); \
        self->end = self->begin + size; \
    } \
} \
 \
void dyn_T_push(dyn_T *self, const struct_t *elem) { \
    if (self->begin == self->end) { \
        dyn_T_realloc(self); \
    } \
    *self->cur++ = elem; \
} \
 \
bool dyn_T_contains(dyn_T *self, const struct_t *query) { \
    for (const struct_t **it = self->begin; it != self->cur; ++it) { \
        if (*it == query) \
            return true; \
    } \
    return false; \
} \

