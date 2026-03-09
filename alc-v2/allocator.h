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

