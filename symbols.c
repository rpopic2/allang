#include <stdio.h>
#include <stdlib.h>

#define INIT_CAP 8

#define list(T) \
    struct list_##T { \
        T *data; \
        int count; \
        int cap; \
    }; \
    void list_new_##T(struct list_ ## T *self) { \
        self->data = malloc(INIT_CAP * sizeof (struct list_ ## T)); \
        self->cap = INIT_CAP; \
        self->count = 0; \
    } \
    void list_add_##T(struct list_ ## T *self, T value) { \
        int cap = self->cap; \
        if (self->count >= cap) { \
            void *tmp = self->data; \
            self->cap *= 2; \
            self->data = reallocf(self->data, cap * sizeof (struct list_##T)); \
            printf("realloc (%p -> %p)\n", tmp, self->data); \
        } \
        self->data[self->count] = value; \
        self->count += 1; \
    } \
    void list_delete_##T(struct list_ ## T *self) { \
        free(self->data); \
    } \

struct symbol {
    char *p;
    uint32_t addr;
};

list(uint32_t)
list(uint64_t)

struct _symbols {
    struct symbol *data;
    int count;
    int cap;
};

typedef struct _resolve_data {
    uint32_t addr;
    char *str;
} resolve_data;
list(resolve_data)


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

