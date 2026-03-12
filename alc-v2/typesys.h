#pragma once

#include "arr.h"
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

enum sign_t {
    S_UNSIGNED, S_SIGNED
};
typedef u8 sign_t;

typedef struct type_t {
    size_t size;
    u8 align;
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
    usize decl_len;
} dtype_t;

static inline void decl_push(dtype_t *self, declarator_t decl) {
    if (self->decl_len >= DECLARATOR_MAX) {
#define XSTR(X) #X
#define DECL_ERROR_STR(X) "there can be only max "XSTR(X)" declarators\n"
        fputs(DECL_ERROR_STR(DECLARATOR_MAX), stderr);
        return;
    }
#undef DECL_ERROR_STR
    self->decl[self->decl_len++] = decl;
}

static inline declarator_t decl_top(const dtype_t *self) {
    if (self->decl_len == 0) {
        return (declarator_t){0};
    }
    return self->decl[self->decl_len - 1];
}

static inline declarator_t decl_pop(dtype_t *self) {
    if (self->decl_len == 0) {
        return (declarator_t){0};
    }
    return self->decl[--self->decl_len];
}

static inline dtype_t decl_dup_strip(dtype_t *self) {
    dtype_t ret = *self;
    decl_pop(&ret);
    return ret;
}

static inline bool decl_empty(const dtype_t *self) {
    return self->decl_len == 0;
}

static inline u32 decl_tryget_arr(const dtype_t *self) {
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

typedef struct reg {
    i32 offset;
    u32 array;
    reg_size rsize;
    register_dst reg_type : 3; // enum register_dst
    u8 addr : 2;
    type_t *type;
    dtype_t dtype;
} reg_t;

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


// end of a block, start of a block
typedef enum {
    EOB_NONE, EOB, SOB
} eob_t;

typedef struct token {
    union {
        struct {
            const char *data;
            const char *end;
        };
        str id;
    };
    unsigned short lineno;
    unsigned char indent;
    eob_t eob;   // end of block
} token_t;

inline str str_from_token(const token_t *token) {
    return (str){ token->data, token->end };
}
#define DEFERRED_NONE 0

#define PARAMS_MAX 16
ARR_GENERIC(str, PARAMS_MAX)
typedef struct {
    reg_t *reg;
    bool target_assigned;
    str name;
} target;

#define MAX_PARAMS 8
ARR_GENERIC(reg_t, MAX_PARAMS)
typedef struct {
    str name;
    u8 airity;
    u8 ret_airity;
    bool is_fn;
    arr_reg_t params;
    arr_reg_t rets;
} symbol_t;

#define MAX_BLOCK_DEPTH 10
ARR_GENERIC(target, MAX_BLOCK_DEPTH)
ARR_GENERIC(u16, MAX_BLOCK_DEPTH)
ARR_GENERIC(u8, MAX_BLOCK_DEPTH)

typedef struct parser_context {
    iter *src;
    reg_t reg;
    int stack_size;
    bool calls_fn;
    bool ended;
    bool has_branched_ret;
    bool last_line_ret;
    u8 indent;
    u8 nreg_count;
    u8 max_nreg_count;
    u16 unnamed_labels;
    symbol_t *last_fn_call;
    str name;
    token_t cur_token;
    arr_target targets;
    arr_u16 deferred_unnamed_br;
    arr_u8 nreg_mark;
    symbol_t *symbol;
} parser_context;

inline static int/*?*/ power_of_two_exponent(size_t n) {
    if (!n || (n & (n - 1)))
        return 0;
    return __builtin_ctzll(n);
}

