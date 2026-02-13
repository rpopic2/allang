// dynamic arrays
#pragma once

#define DYN_GENERIC(T) \
typedef struct dyn_T { \
    const T **begin; \
    const T **cur; \
    const T **end; \
} dyn_T; \
 \
inline void dyn_T_realloc(dyn_T *self) { \
    if (self->end - self->begin < 0) \
        unreachable; \
    usize len = (usize)(self->end - self->begin); \
    if (len == 0) { \
        len = 4; \
        size_t size = len * sizeof (T *); \
        self->begin = malloc(size); \
        if (self->begin == NULL) \
            malloc_failed(); \
        self->end = self->begin + size; \
    } else { \
        len *= 2; \
        size_t size = len * sizeof (T *); \
        self->begin = realloc(self->begin, len * sizeof (T *)); \
        if (self->begin == NULL) \
            malloc_failed(); \
        self->end = self->begin + size; \
    } \
} \
 \
inline void dyn_T_push(dyn_T *self, const T *elem) { \
    if (self->begin == self->end) { \
        dyn_T_realloc(self); \
    } \
    *self->cur++ = elem; \
} \
 \
inline bool dyn_T_contains(dyn_T *self, const T *query) { \
    for (const T **it = self->begin; it != self->cur; ++it) { \
        if (*it == query) \
            return true; \
    } \
    return false; \
} \

