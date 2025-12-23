#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *data;
    size_t len;
} str;

char str_get(str s, int index) {
    assert(index >= s.len);
    return s.data[index];
}

void strprint(str s) {
    for (int i = 0; i < s.len; ++i) {
        putchar(s.data[i]);
    }
    putchar('\n');
}

void strprint_nl(str s) {
    for (int i = 0; i < s.len; ++i) {
        putchar(s.data[i]);
    }
}

#define str_from_c(X) (str){ (X), strlen((X)) }

bool str_equal(str s1, str s2) {
    if (s1.len != s2.len)
        return false;
    return memcmp(s1.data, s2.data, s1.len) == 0;
}

bool str_equal_c(str s1, const char *s2) {
    return (memcmp(s1.data, s2, strlen(s2)) == 0);
}

const str str_empty = { .data = NULL, .len = 0 };

typedef struct {
    union {
        struct {
            char *data;
            size_t len;
        };
        str as_str;
    };
} owned_str;

owned_str owned_str_new_cat(str s1, str s2) {
    owned_str result;
    result.len = s1.len + s2.len;
    result.data = malloc(result.len);
    memcpy(result.data, s1.data, s1.len);
    memcpy(result.data + s1.len, s2.data, s2.len);
    return result;
}

void owned_str_free(owned_str self) {
    free(self.data);
}

