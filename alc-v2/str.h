#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

 void abort(void);

typedef struct str {
    const char *data;
    const char *end;
} str;

typedef struct {
    char *cur;
    char *start;
    char *end;
} iter;

static const str str_null = {.data = 0, .end = 0};

inline size_t str_len(const str *s) {
    if (s->end - s->data < 0)
        abort();
    return (size_t)(s->end - s->data);
}

inline bool str_is_empty(const str *s) {
    return s->end == s->data;
}

inline bool str_eq_lit(const str *restrict s, const char *restrict cstr) {
    return memcmp(s->data, cstr, str_len(s)) == 0;
}

static inline str str_move(str *s) {
    str ret = *s;
    *s = str_null;
    return ret;
}

inline static bool str_ends_with(const str *restrict token, const char *restrict cstr) {
    size_t len = strlen(cstr);
    return str_len(token) >= len && memcmp(token->end - len, cstr, len) == 0;
}

static inline void str_fprintnl(const str *s, FILE *file) {
    if (s->data == s->end) {
        fputs("(empty)", file);
    } else {
        if (str_len(s) > 0)
            fwrite(s->data, sizeof (char), (size_t)str_len(s), file);
    }
}

static inline void str_fprint(const str *s, FILE *file) {
    str_fprintnl(s, file);
    fputc('\n', file);
}

static inline void str_print(const str *s) {
    str_fprintnl(s, stdout);
    fputc('\n', stdout);
}

inline str str_from_iter(const iter *it) {
    return (str){.data = it->start, it->end};
}

inline iter iter_init(char *start, size_t end) {
    return (iter){.start = start, .cur = start, .end = start + end};
}

#define STR_FROM(s) (str) { .data = (s), .end = (s) + strlen(s) }

