#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct allocator {
    char *data;
    char *end;
    size_t size;
} allocator;

void allocator_init(allocator *self, void *data, size_t size) {
    *self = (allocator){
        .data = data,
        .size = size,
        .end = data,
    };
}

void *allocator_alloc_undefined(allocator *self, size_t size) {
    void *ret = self->end;
    self->end += size;
    if (self->end >= (self->data + self->size)) {
        fprintf(stderr, "allocation failed!\n");
        abort();
    }
    return ret;
}

void *allocator_alloc(allocator *self, size_t size) {
    void *ret = allocator_alloc_undefined(self, size);
    memset(ret, 0, size);
    return ret;
}

// fixed dynamic arrays
#pragma once

#define list_GENERIC(T) \
typedef struct list_##T { \
    T *begin; \
    T *cur; \
    T *end; \
    allocator *alloc; \
} list_##T; \
 \
inline static void list_##T##_init(list_##T *self, allocator *alloc, size_t len) { \
    self->allocator = alloc; \
    size_t size = sizeof (T) * len; \
    self->begin = allocator_alloc_undefined(alloc, size); \
    self->cur = self->begin; \
    self->end = self->begin + len; \
} \
 \
inline static void list_##T##_push(list_##T *self, const T *elem) { \
    if (self->begin == self->cur) { \
        fprintf(stderr, "array was full\n"); \
        abort() \
    } \
    *(self->cur++) = *elem; \
} \
 \
inline static ptrdiff_t list_##T##_len(list_##T *self) { \
    return self->cur - self->begin; \
} \
