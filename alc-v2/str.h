#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct str {
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
    return memcmp(s->data, cstr, str_len(s)) == 0;
}

inline void str_fprintnl(const str *s, FILE *file) {
    if (s->data == s->end) {
        fputs("(empty)", file);
    } else {
        fwrite(s->data, sizeof (char), str_len(s), file);
    }
}

inline void str_fprint(const str *s, FILE *file) {
    str_fprintnl(s, file);
    fputc('\n', file);
}

inline void str_print(const str *s) {
    str_fprintnl(s, stdout);
    fputc('\n', stdout);
}

inline bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

inline iter iter_init(char *start, size_t end) {
    return (iter){.start = start, .cur = start, .end = start + end};
}

#define STR_FROM(s) (str) { .data = (s), .end = (s) + strlen(s) }

