#pragma once

#include "typedefs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INIT_CAP 8

#define ls(T) \
    typedef struct { \
        T *data; \
        int count; \
        int cap; \
        char *debug_name; \
    } ls_##T; \
    void ls_new_##T(ls_## T *self, int init_cap, char *debug_name) { \
        self->data = malloc(init_cap * sizeof (T)); \
        self->cap = init_cap; \
        self->count = 0; \
        self->debug_name = debug_name; \
    } \
    void ls_reserv_##T(ls_## T *self, size_t newcap) { \
        while (self->cap <= newcap) { \
            printf("%s realloc cap(%zx) (%x ->", self->debug_name, newcap, self->cap); \
            self->cap = self->cap << 2; \
            printf(" %x)\n", self->cap); \
            self->data = reallocf(self->data, self->cap * sizeof (T)); \
        } \
    } \
    void ls_add_##T(ls_## T *self, T value) { \
        ls_reserv_##T(self, self->count + 1); \
        self->data[self->count] = value; \
        self->count += 1; \
    } \
    void ls_delete_##T(ls_## T *self) { \
        free(self->data); \
    } \
    void ls_addran_##T(ls_## T *restrict self, const T *restrict ptr, size_t nitems) { \
        ls_reserv_##T(self, self->count + nitems); \
        memcpy(self->data + self->count, ptr, nitems * sizeof (T)); \
        self->count += nitems; \
    }

ls(char)
ls(int)
ls(u32)



typedef struct {
  void **data;
  int count;
  int cap;
  char *debug_name;
} ls_voidp;
void ls_new_voidp(ls_voidp *self, int init_cap, char *debug_name) {
  self->data = malloc(init_cap * sizeof(void *));
  self->cap = init_cap;
  self->count = 0;
  self->debug_name = debug_name;
}
void ls_reserv_voidp(ls_voidp *self, size_t newcap) {
  while (self->cap <= newcap) {
    printf("%s realloc cap(%zx) (%x ->", self->debug_name, newcap, self->cap);
    self->cap = self->cap << 2;
    printf(" %x)\n", self->cap);
    self->data = reallocf(self->data, self->cap * sizeof(void *));
  }
}
void ls_add_voidp(ls_voidp *self, void * value) {
  ls_reserv_voidp(self, self->count + 1);
  self->data[self->count] = value;
  self->count += 1;
}
void ls_delete_voidp(ls_voidp *self) { free(self->data); }
void ls_addran_voidp(ls_voidp *restrict self, const void **restrict ptr,
                   size_t nitems) {
  ls_reserv_voidp(self, self->count + nitems);
  __builtin___memcpy_chk(self->data + self->count, ptr, nitems * sizeof(void *),
                         __builtin_object_size(self->data + self->count, 0));
  self->count += nitems;
}
