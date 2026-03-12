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
    TK_NONE, TK_FUND, TK_STRUCT, TK_UNION,
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
            dyn_member_t members;
        } struct_t;
    };
} type_t;

enum dtype_kind {
    DK_ADDR, DK_ARRAY
};

typedef struct delarator {
    enum dtype_kind tag : 1;    // enum dtype_kind
    u32 amount : 31;
} declarator_t;

// derived type
#define DECLARATOR_MAX 8
typedef struct dtype {
    type_t *base;
    declarator_t decl[DECLARATOR_MAX];
    usize deco_len;
} dtype_t;

static inline void decl_push(dtype_t *self, declarator_t decl) {
    if (self->deco_len >= DECLARATOR_MAX) {
#define XSTR(X) #X
#define DECL_ERROR_STR(X) "there can be only max "XSTR(X)" declarators\n"
        fputs(DECL_ERROR_STR(DECLARATOR_MAX), stderr);
        return;
    }
#undef DECL_ERROR_STR
    self->decl[self->deco_len++] = decl;
}

static inline declarator_t decl_top(dtype_t *self) {
    if (self->deco_len == 0) {
        return (declarator_t){0};
    }
    return self->decl[self->deco_len - 1];
}

static inline declarator_t decl_pop(dtype_t *self) {
    if (self->deco_len == 0) {
        return (declarator_t){0};
    }
    return self->decl[--self->deco_len];
}

static inline bool decl_empty(dtype_t *self) {
    return self->deco_len == 0;
}

static inline u32 decl_tryget_arr(dtype_t *self) {
    declarator_t top = decl_top(self);
    if (top.tag != DK_ARRAY)
        return 0;
    else return top.amount;
}

static inline size_t dtype_size(dtype_t *self) {
    if (decl_empty(self)) {
        return self->base->size;
    }
    declarator_t top = decl_top(self);
    if (top.tag == DK_ADDR)
        return sizeof (void *);
    else if (top.tag == DK_ARRAY) {
        return self->base->size * top.amount;
    } else {
        unreachable;
    }
}

enum tag {
    NONE, VALUE, REG, AGGREGATE
};

typedef struct {
    union {
        struct {
            i64 value;
            type_t *type;
        };
        reg_t reg;
    };
    u8 tag;
} regable;
DYN_GENERIC(regable)

typedef struct dyn_agg_member dyn_agg_member;
typedef struct {
    union {
        i64 value;
        reg_t reg;
        dyn_agg_member *agg;
    };
    u8 tag;
} agg_member;
DYN_GENERIC(agg_member)

static inline agg_member agg_member_from(const regable *r) {
    agg_member a = {0};
    a.tag = r->tag;
    if (r->tag == VALUE) {
        a.value = r->value;
    } else if (r->tag == REG) {
        a.reg = r->reg;
    } else if (r->tag == NONE) {

    } else {
        unreachable;
    }
    return a;
}

