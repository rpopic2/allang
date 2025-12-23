#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

typedef struct {
    const char *data;
    const char *end;
} str;

typedef struct {
    char *cur;
    char *start;
    char *end;
} iter;

inline ptrdiff_t str_len(const str *s) {
    return s->end - s->data;
}

inline bool str_eq_lit(const str *restrict s, const char *restrict cstr) {
    return strcmp(s->data, cstr) == 0;
}

inline bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

#define STR_FROM_CSTR(s) (str) { .data = (s), .end = (s) + strlen(s) }

