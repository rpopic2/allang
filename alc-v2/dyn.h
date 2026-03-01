// dynamic arrays
#pragma once

#define DYN_GENERIC(T) \
typedef struct dyn_##T { \
    T *begin; \
    T *cur; \
    T *end; \
} dyn_##T; \
 \
inline static void dyn_##T##_realloc(dyn_##T *self) { \
    if (self->end - self->begin < 0) \
        unreachable; \
    usize capa = (usize)(self->end - self->begin); \
    if (capa == 0) { \
        capa = 8; \
        self->begin = calloc(capa, sizeof (T)); \
        if (self->begin == NULL) \
            malloc_failed(); \
        self->end = self->begin + capa; \
        self->cur = self->begin; \
    } else { \
        usize len = (usize)(self->cur - self->begin); \
        capa *= 2; \
        size_t size = capa * sizeof (T); \
        self->begin = realloc(self->begin, size); \
        if (self->begin == NULL) \
            malloc_failed(); \
        self->end = self->begin + capa; \
        self->cur = self->begin + len; \
    } \
} \
 \
inline static void dyn_##T##_reserve(dyn_##T *self, ptrdiff_t len) { \
    while (self->end - self->begin < len) { \
        dyn_##T##_realloc(self); \
    } \
} \
 \
inline static void dyn_##T##_push(dyn_##T *self, const T *elem) { \
    if (self->begin == self->cur) { \
        dyn_##T##_realloc(self); \
    } \
    *(self->cur++) = *elem; \
} \
 \
inline static void dyn_##T##_free(dyn_##T *self) { \
    free(self->begin); \
    *self = (dyn_##T){0}; \
} \
 \
inline static ptrdiff_t dyn_##T##_len(dyn_##T *self) { \
    return self->cur - self->begin; \
} \
