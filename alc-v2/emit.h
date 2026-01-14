#pragma once

#include <stdio.h>
#include <stdbool.h>

#include "arr.h"
#include "str.h"
#include "types.h"

// end of a block, start of a block
typedef enum {
    EOB_NONE, EOB, SOB
} eob_t;

typedef struct {
    union {
        struct {
            const char *data;
            const char *end;
        };
        str id;
    };
    unsigned char lineno;
    unsigned char indent;
    eob_t eob;   // end of block
} token_t;

inline str str_from_token(const token_t *token) {
    return (str){ token->data, token->end };
}

typedef struct {
    reg_t *reg;
    bool target_assigned;
} target;

#define MAX_PARAMS 8
ARR_GENERIC(reg_t, MAX_PARAMS)
typedef struct {
    str name;
    u8 airity;
    u8 ret_airity;
    bool is_fn;
    arr_reg_t params;
} symbol_t;

#define MAX_BLOCK_DEPTH 10
ARR_GENERIC(target, MAX_BLOCK_DEPTH)
ARR_GENERIC(u16, MAX_BLOCK_DEPTH)

typedef struct {
    iter *src;
    reg_t reg;
    int nreg_count;
    int stack_size;
    bool calls_fn;
    bool ended;
    bool has_branched_ret;
    bool last_line_ret;
    u8 indent;
    u16 unnamed_labels;
    symbol_t *deferred_fn_call;
    str name;
    token_t cur_token;
    arr_target targets;
    arr_u16 deferred_unnamed_br;
    symbol_t *symbol;
} parser_context;
#define DEFERRED_NONE 0

#define PARAMS_MAX 16
ARR_GENERIC(str, PARAMS_MAX)

// typedef struct {
//     str fn_name;
//     bool is_tmp;
//     union {
//         str name;
//         int index;
//     };
// } label_t;

void emit_init(void);
void emit_reset_fn(void);
void emit_fnbuf(FILE *out);
void emit_text(FILE *out);
void emit_cstr(FILE *out);

bool emit_need_escaping(void);

void emit_mov(reg_t dst, i64 value);
void emit_mov_reg(reg_t dst, reg_t src);
void emit_add(reg_t dst, reg_t lhs, i64 rhs);
void emit_add_reg(reg_t dst, reg_t lhs, reg_t rhs);
void emit_sub(reg_t dst, reg_t lhs, i64 rhs);
void emit_sub_reg(reg_t dst, reg_t lhs, reg_t rhs);
void emit_cmp(reg_t lhs, i64 rhs);
void emit_cmp_reg(reg_t lhs, reg_t rhs);
void emit_string_lit(reg_t dst, const str *s);

void emit_str(reg_t src, reg_t dst, int offset);
void emit_ldr(reg_t dst, reg_t src, int offset);
void emit_str_reg(reg_t src, reg_t dst, reg_t offset);
void emit_ldr_reg(reg_t dst, reg_t src, reg_t offset);

void emit_branch(str fn_name, str label, int index);
bool emit_branch_cond(cond condition, str fn_name, str label, int index);
void emit_label(str fn_name, str label, int index);
void emit_fn_prologue_epilogue(const parser_context *context);
void emit_fn_call(const str *s);
void emit_fn(str fn_name);
void emit_ret(void);

extern const char *text_section_header;
extern const char *string_section_header;
