#include "dyn.h"
#include "str.h"
#include "types.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct member {
    type_t *type;
    str name;
    size_t offset;
} member_t;

DYN_GENERIC(member_t)

enum type_kind {
    TK_NONE, TK_FUND, TK_ARRAY, TK_STRUCT, TK_UNION,
};
typedef u8 type_kind;

typedef struct type_t {
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
            dyn_member_t members;
        } struct_t;
    };
} type_t;

typedef struct {
    enum {
        NONE, VALUE, REG
    } tag;
    union {
        i64 value;
        reg_t reg;
    };
} regable;
DYN_GENERIC(regable)
