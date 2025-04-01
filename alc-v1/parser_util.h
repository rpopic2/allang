#pragma once
#include "file.h"
#include "list.h"
#include "slice.h"
#include "str.h"
#include <stdbool.h>

static inline bool IsNum(char X) {
    return X >= '0' && X <= '9';
}

static inline bool IsUpper(char X) {
    return X >= 'A' && X <= 'Z';
}

static inline bool IsLower(char X) {
    return X >= 'a' && X <= 'z';
}

static inline bool IsAlpha(char X) {
    return IsLower(X) || IsUpper(X);
}

static inline bool IsSpace(char c) {
    return c == ' ' || c == '\n';
}

#define is ==
#define or ||

#define Next() iter_next(&it)
#define Is(X) _if_is(X, &it, &c)

#define ReadUntilSpace() while (c != ' ' && c != '\n' && c != '\0') { c = Next(); }
#define ReadToken_old() while (c != ' ' && c != '\n' && c != ',' && c != '"' && c != '\0' && c != '=') { c = Next(); }

#define ReadToken while (IsAlpha(c) || IsNum(c) || c is '.' || c is '_') { c = Next();}

#define printdbg(...) printf(__VA_ARGS__)

#define TokenStart token.data = it.data;
#define TokenEnd  token.len = it.data - token.data;

u32 _objcode[1024];
writer_t objcode = _objcode;

ls_char strings;
bool main_defined;

static inline bool _if_is(const char *s, str_iter *it, int *c) {
    if (memcmp((s), it->data, strlen(s)) == 0) {
        it->data += strlen(s); *c = *it->data;
        return true;
    }
    return false;
}

void parse_scope(str src);
