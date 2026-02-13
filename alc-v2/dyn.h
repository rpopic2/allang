// dynamic arrays
#pragma once

#define DYN_GENERIC(T) \
typedef struct dyn_T { \
    T *begin; \
    T *cur; \
    T *end; \
} dyn_T; \
 \
inline static void dyn_T_realloc(dyn_T *self) { \
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
        self->cur = self->begin; \
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
inline static void dyn_T_push(dyn_T *self, const T *elem) { \
    if (self->begin == self->cur) { \
        dyn_T_realloc(self); \
    } \
    *(self->cur++) = *elem; \
} \
 \
