#pragma once
#include "file.h"
#include "list.h"
#include "slice.h"
#include "stack_context.h"
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
#define isnt !=
#define or ||

#define Next() iter_next(&it)
#define Is(X) _if_is(X, &it, &c)

#define ReadUntilSpace() while (c != ' ' && c != '\n' && c != '\0') { c = Next(); }
#define ReadUntilNewline() while (c != '\n' && c != '\0') { c = Next(); }
#define ReadToken_old() while (c != ' ' && c != '\n' && c != ',' && c != '"' && c != '\0' && c != '=') { c = Next(); }

#define ReadToken while (IsAlpha(c) || IsNum(c) || c is '.' || c is '_') { c = Next();}


#define TokenStart token.data = it.data;
#define TokenEnd  token.len = it.data - token.data;

#define Align0x10(X) ((X) + 0xf) & ~0xf

u32 _objcode[1024];
writer_t objcode = _objcode;

ls_char strings;
bool main_defined;
int depth = 0;

typedef struct {
    str name;
    u32 offset;
} resolv; // entries to resolv (cbz..etc)
ls (resolv);

ls_resolv resolves;

nreg *target_nreg;
int reg_types[32];

static inline bool _if_is(const char *s, str_iter *it, int *c) {
    if (memcmp((s), it->data, strlen(s)) == 0) {
        it->data += strlen(s); *c = *it->data;
        return true;
    }
    return false;
}

void parse_scope(str src);

nreg *nreg_find(stack_context *s, str token) {
    nreg *find = NULL;
    for (int i = 0; i < s->named_regs.count; ++i) {
        nreg *tmp = s->named_regs.data + i;
        if (str_equal(tmp->name, token)) {
            find = tmp;
        }
    }
    return find;
}

obj *obj_find(stack_context *s, str token) {
    obj *find = NULL;
    for (int i = 0; i < s->objects.count; ++i) {
        obj *tmp = s->objects.data + i;
        if (str_equal(tmp->name, token)) {
            find = tmp;
        }
    }
    return find;
}

bool is_target_nreg(str_iter *it) {
    return (target_nreg != NULL && it->data[0] == '\n');
}

