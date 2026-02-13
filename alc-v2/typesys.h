#include "dyn.h"
#include "str.h"
#include "types.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

DYN_GENERIC(type_t)

enum type_kind {
    TK_NONE, TK_FUND, TK_ARRAY, TK_STRUCT, TK_UNION,
};
typedef u8 type_kind;

typedef struct _type_t {
    size_t size;
    u8 align;
    u8 addr;
    type_kind tag;
    sign_t sign;
    str name;
    union {
        struct {
            usize len;
            type_t *type;
        } arr;
        struct {
            dyn_T members;
        } struct_t;
    };
} type_t;

