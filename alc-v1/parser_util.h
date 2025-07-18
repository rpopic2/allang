#pragma once
#include "error.h"
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

#define ReadToken while (IsToken(c)) { c = Next();}
#define IsToken(c) IsAlpha(c) || IsNum(c) || c is '.' || c is '_'


#define TokenStart token.data = it.data;
#define TokenEnd  token.len = it.data - token.data;

#define Align0x10(X) ((X) + 0xf) & ~0xf

u32 _objcode[1024];
writer_t objcode = _objcode;

// ls_char strings;
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

void parse_scope(str src, bool has_params, str name);

nreg *nreg_find(ls_nreg *s, str token) {
    nreg *find = NULL;
    for (int i = 0; i < s->count; ++i) {
        nreg *tmp = s->data + i;
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

int read_int(str token, str_iter *rit) {
    bool is_minus = false;
    str_iter it = *rit;
    char c = *it.data;
    int number = 0;

    if (!IsNum(c) && c != '-') {
        CompileErr("Compile Error: Number expected, was %d", c);
    }

    TokenStart;
    ReadToken;
    TokenEnd;

    // printd("("), strprint_nl(token), printd(")%d)", c);
    if (c is '-' && IsNum(token.data[1])) {
        printd("minus..");
        is_minus = true;
        c = Next();
        TokenStart;
        ReadToken;
        TokenEnd;
        c = token.data[0];
        strprint(token);
    }
    number = strtol(token.data, &it.data, 10);
    if (is_minus) {
        number = -number;
    }
    *rit = it;
    return number;
}

char *find_line_end(char *p) {
    while (*p != '\n' && *p != '\0') {
        ++p;
    }
    return p - 1;
}

bool should_use_x0_reg(char *line_end, str src, str_iter *it) {
    return line_end + 3 >= (src.data + src.len)    // is end of file
            || (*line_end == '>' && line_end[-1] != '>' && line_end[-2] != ' ') // line ends with => or ->
            || (str_equal_c((str){it->data + 1, 3}, "ret")); // line ends with ret
}
