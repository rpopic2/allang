#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typedefs.h"

#define INIT_CAP 8

#define ls(T) \
    typedef struct { \
        T *data; \
        int count; \
        int cap; \
        char *debug_name; \
    } ls##T; \
    void ls_new_##T(ls ## T *self, int init_cap, char *debug_name) { \
        self->data = malloc(init_cap * sizeof (T)); \
        self->cap = init_cap; \
        self->count = 0; \
        self->debug_name = debug_name; \
    } \
    void ls_reserv_##T(ls ## T *self, size_t newcap) { \
        while (self->cap <= newcap) { \
            printf("%s realloc cap(%zx) (%x ->", self->debug_name, newcap, self->cap); \
            self->cap = self->cap << 2; \
            printf(" %x)\n", self->cap); \
            self->data = reallocf(self->data, self->cap * sizeof (T)); \
        } \
    } \
    void ls_add_##T(ls ## T *self, T value) { \
        ls_reserv_##T(self, self->count + 1); \
        self->data[self->count] = value; \
        self->count += 1; \
    } \
    void ls_delete_##T(ls ## T *self) { \
        free(self->data); \
    } \
    void ls_addrang##T(ls## T *restrict self, T *restrict ptr, size_t nitems) { \
        ls_reserv_##T(self, self->count + nitems); \
        memcpy(self->data + self->count, ptr, nitems * sizeof (T)); \
        self->count += nitems; \
    } \

ls (u32);
