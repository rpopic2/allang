#pragma once

#include <stdio.h>
#include <stdbool.h>

#include "arr.h"
#include "str.h"
#include "types.h"

typedef struct {
    const char *data;
    const char *end;
    int lineno;
    int indent;
    bool eob;   // end of block
} token_t;

typedef struct {
    reg_t *reg;
    bool target_assigned;
} target;

ARR_GENERIC(target, 10)

typedef struct {
    reg_t reg;
    int nreg_count;
    str deferred_fn_call;
    iter src;
    bool calls_fn;
    int stack_size;
    token_t cur_token;
    arr_target targets;
} parser_context;

void emit_init(void);
void emit(FILE *out);

bool emit_need_escaping(void);

void emit_mov(register_dst reg_dst, int regidx, i64 value);
void emit_mov_reg(register_dst reg_dst, int regidx, register_dst reg_src, int regidx_src);
void emit_add(reg_t dst, reg_t lhs, i64 rhs);
void emit_add_reg(reg_t dst, reg_t lhs, reg_t rhs);
void emit_sub(reg_t dst, reg_t lhs, i64 rhs);
void emit_sub_reg(reg_t dst, reg_t lhs, reg_t rhs);
void emit_string_lit(register_dst reg_dst, int regidx, const str *s);

void emit_str_fp(reg_t src, int offset);
void emit_ldr_fp(reg_t dst, int offset);

void emit_fn_prologue_epilogue(const parser_context *context);
void emit_fn_call(const str *s);
void emit_mainfn(void);
void emit_ret(void);

extern const char *text_section_header;
extern const char *string_section_header;
