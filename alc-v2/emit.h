#pragma once

#include <stdio.h>
#include <stdbool.h>

#include "arr.h"
#include "str.h"
#include "types.h"

typedef struct reg reg_t;
typedef struct dtype dtype_t;
typedef struct dyn_agg_member dyn_agg_member;
typedef struct dyn_regable dyn_regable;
typedef struct parser_context parser_context;

void emit_init(void);
void emit_reset_fn(void);
void emit_fnbuf(FILE *out);
void emit_text(FILE *out);
void emit_cstr(FILE *out);

bool emit_need_escaping(void);

void emit_make_struct(reg_t dst, dtype_t *dtype, dyn_agg_member *args);
void emit_store_struct(reg_t dst, i64 offset, dtype_t *dtype, dyn_agg_member *args);
void emit_make_array(reg_t dst, type_t *type, u32 len, dyn_regable *args);
void emit_store_array(reg_t dst, i64 offset, type_t *type, u32 len, dyn_regable *args);
void emit_array_access(reg_t dst, reg_t src, reg_t offset);

void emit_mov(reg_t dst, i64 value);
void emit_mov_reg(reg_t dst, reg_t src);
void emit_add(reg_t dst, reg_t lhs, i64 rhs);
void emit_add_reg(reg_t dst, reg_t lhs, reg_t rhs);
void emit_sub(reg_t dst, reg_t lhs, i64 rhs);
void emit_sub_reg(reg_t dst, reg_t lhs, reg_t rhs);
void emit_cmp(reg_t lhs, i64 rhs);
void emit_cmp_reg(reg_t lhs, reg_t rhs);
void emit_string_lit(reg_t dst, const str *s);
void emit_lsl(reg_t dst, reg_t lhs, i64 rhs);

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

void report_error(const char *format, ...);

extern const char *text_section_header;
extern const char *string_section_header;
extern const size_t default_register_size;
