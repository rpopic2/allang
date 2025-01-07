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
    }; \
    void list_new_##T(struct list_ ## T *self) { \
        self->data = malloc(INIT_CAP * sizeof (T)); \
        self->cap = INIT_CAP; \
        self->count = 0; \
    } \
    void list_reserv_##T(struct list_ ## T *self, size_t newcap) { \
        while (self->cap < newcap) { \
            printf("realloc cap (%d -> %zx)\n", self->cap, newcap); \
            self->cap *= 2; \
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


enum symbol_type {
    unknwon, code, code_undef, stack_obj
};

struct symbol {
    char *p;
    uint32_t addr;
    enum symbol_type type;
};

list (uint32_t)
list (uint64_t)

struct _symbols {
    struct symbol *data;
    int count;
    int cap;
};

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

void symbols_new(struct _symbols *self) {
    self->data = malloc(INIT_CAP * sizeof (struct _symbols));
    self->cap = INIT_CAP;
    self->count = 0;
}
void symbols_add(struct _symbols *self, struct symbol target) {
    int cap = self->cap;
    if (self->count >= cap) {
        self->data = reallocf(self->data, cap * 2);
        printf("realloc\n");
    }
    self->data[self->count] = target;
    self->count += 1;
}

void symbols_delete(struct _symbols *self) {
    free(self->data);
}

