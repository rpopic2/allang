#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INIT_CAP 8

#define list(T) \
    struct list_##T { \
        T *data; \
        int count; \
        int cap; \
        char *debug_name; \
    }; \
    void list_new_##T(struct list_ ## T *self, int init_cap, char *debug_name) { \
        self->data = malloc(init_cap * sizeof (T)); \
        self->cap = init_cap; \
        self->count = 0; \
        self->debug_name = debug_name; \
    } \
    void list_reserv_##T(struct list_ ## T *self, size_t newcap) { \
        while (self->cap <= newcap) { \
            printf("%s realloc cap(%zx) (%x ->", self->debug_name, newcap, self->cap); \
            self->cap = self->cap << 2; \
            printf(" %x)\n", self->cap); \
            self->data = reallocf(self->data, self->cap * sizeof (T)); \
        } \
    } \
    void list_add_##T(struct list_ ## T *self, T value) { \
        list_reserv_##T(self, self->count + 1); \
        self->data[self->count] = value; \
        self->count += 1; \
    } \
    void list_delete_##T(struct list_ ## T *self) { \
        free(self->data); \
    } \
    void list_addrang_##T(struct list_## T *restrict self, T *restrict ptr, size_t nitems) { \
        list_reserv_##T(self, self->count + nitems); \
        memcpy(self->data + self->count, ptr, nitems * sizeof (T)); \
        self->count += nitems; \
    } \


list (uint32_t)
list (uint64_t)

enum symbol_type {
    unknwon, code, code_undef, stack_obj
};
typedef struct symbol {
    char *p;
    uint32_t addr;
    enum symbol_type type;
} symbol_t;
list (symbol_t)

typedef struct _resolve_data {
    uint32_t addr;
    char *str;
} resolve_data;
list (resolve_data)

typedef struct _object {
    char *name;
    int16_t offset;
    uint16_t size;
    bool sign;
} object;
list (object)

