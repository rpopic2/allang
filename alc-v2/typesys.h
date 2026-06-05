#pragma once

#include "arr.h"
#include "dyn.h"
#include "str.h"
#include "types.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct dtype dtype_t;
typedef struct emit_context emit_context_t;

enum type_kind {
    TK_NONE, TK_FUND, TK_STRUCT, TK_UNION,
};
typedef u8 type_kind;

enum sign_t {
    S_UNSIGNED, S_SIGNED
};
typedef u8 sign_t;

enum dtype_kind {
    DK_ADDR, DK_ARRAY, DK_CHECK, DK_SLICE
};

typedef struct delarator {
    enum dtype_kind tag : 2;
    i32 amount : 30;
} declarator_t;

#define DECLARATOR_MAX 8
typedef struct dtype {
    type_t *base;
    declarator_t decl[DECLARATOR_MAX];
    usize decl_len;
} dtype_t;

static inline bool dtype_eq(const dtype_t *lhs, const dtype_t *rhs) {
    if (lhs->decl_len != rhs->decl_len)
        return false;

    if (memcmp(&lhs->decl, &rhs->decl, sizeof lhs->decl) != 0)
        return false;

    bool base_eq = lhs->base == rhs->base;
    return base_eq;
}

typedef struct member {
    dtype_t dtype;
    str name;
    size_t offset;
} member_t;

DYN_GENERIC(member_t)

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

// derived type

static inline void dtype_push(dtype_t *self, declarator_t decl) {
    if (self->decl_len >= DECLARATOR_MAX) {
#define XSTR(X) #X
#define DECL_ERROR_STR(X) "there can be only max "XSTR(X)" declarators\n"
        fputs(DECL_ERROR_STR(DECLARATOR_MAX), stderr);
        return;
    }
#undef DECL_ERROR_STR
    self->decl[self->decl_len++] = decl;
}

static inline declarator_t dtype_top(const dtype_t *self) {
    if (self->decl_len == 0) {
        return (declarator_t){0};
    }
    return self->decl[self->decl_len - 1];
}

static inline declarator_t dtype_pop(dtype_t *self) {
    if (self->decl_len == 0) {
        return (declarator_t){0};
    }
    return self->decl[--self->decl_len];
}

static inline dtype_t dtype_dup_strip(dtype_t *self) {
    dtype_t ret = *self;
    dtype_pop(&ret);
    return ret;
}

static inline bool dtype_empty(const dtype_t *self) {
    return self->decl_len == 0;
}

static inline i32 dtype_tryget_arr(const dtype_t *self) {
    declarator_t top = dtype_top(self);
    if (top.tag != DK_ARRAY)
        return 0;
    else return top.amount;
}

static inline i32 dtype_tryget_addr(const dtype_t *self) {
    declarator_t top = dtype_top(self);
    if (top.tag != DK_ADDR)
        return 0;
    else return top.amount;
}

static inline size_t dtype_size(const dtype_t *self) {
    if (dtype_empty(self)) {
        return self->base->size;
    }
    declarator_t top = dtype_top(self);
    if (top.tag == DK_ADDR)
        return sizeof (void *);
    else if (top.tag == DK_ARRAY) {
        if (top.amount <= 0)
            fprintf(stderr, "array length was <= 0 (%d)", top.amount);
        return self->base->size * (usize)top.amount;
    } else if (top.tag == DK_CHECK) {
        dtype_t stripped = *self;
        dtype_pop(&stripped);
        return dtype_size(&stripped);
    } else {
        unreachable;
    }
}

static inline int dtype_reg_count(const dtype_t *self) {
    for (usize i = 0; i < self->decl_len; ++i) {
        if (self->decl[i].tag == DK_SLICE)
            return 2;
    }
    return 1;
}

typedef struct reg {
    i32 offset;
    reg_size rsize;
    register_dst reg_type : 3; // enum register_dst
    dtype_t dtype;
} reg_t;

static inline bool reg_eq(struct reg lhs, struct reg rhs) {
    return lhs.offset == rhs.offset && lhs.rsize == rhs.rsize && lhs.reg_type == rhs.reg_type && dtype_eq(&lhs.dtype, &rhs.dtype);
}

enum tag {
    NONE, VALUE, REG, AGGREGATE
};

typedef struct {
    union {
        struct {
            i64 value;
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
    const char *filename;
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
    bool is_called;
    bool is_placeholder;
    arr_reg_t params;
    arr_reg_t rets;
} symbol_t;

#define MAX_BLOCK_DEPTH 10
ARR_GENERIC(target, MAX_BLOCK_DEPTH)
ARR_GENERIC(u16, MAX_BLOCK_DEPTH)
ARR_GENERIC(u8, MAX_BLOCK_DEPTH)

typedef struct {
    str name;
    str type_name;
    size_t offset;
    size_t size;
} stack_slot_t;
#define MAX_STACK_SLOTS 64

typedef struct src {
    char *cur;
    char *start;
    char *end;
    char filename[FILENAME_MAX];
} src_t;

typedef struct parser_context {
    src_t *src;
    reg_t reg;
    int stack_size;
    u16 unnamed_labels;
    bool calls_fn;
    bool ended;
    bool has_branched_ret;
    bool last_line_ret;
    u8 indent;
    u8 nreg_count;
    u8 max_nreg_count;
    bool end_of_line;
    bool start_of_line;
    symbol_t *last_fn_call;
    str name;
    token_t cur_token;
    arr_target targets;
    arr_u16 deferred_unnamed_br;
    arr_u8 nreg_mark;
    symbol_t *symbol;
    stack_slot_t stack_slots[MAX_STACK_SLOTS];
    int stack_slot_count;
} parser_context;

inline static int/*?*/ power_of_two_exponent(size_t n) {
    if (!n || (n & (n - 1)))
        return 0;
    return __builtin_ctzll(n);
}
