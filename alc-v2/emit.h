#pragma once

#include <stdio.h>
#include <stdbool.h>

#include "arr.h"
#include "str.h"
#include "types.h"
#include "buffer.h"

typedef struct reg reg_t;
typedef struct dtype dtype_t;
typedef struct dyn_agg_member dyn_agg_member;
typedef struct dyn_regable dyn_regable;
typedef struct parser_context parser_context;

typedef struct emit_context {
    buf fn_header_buf;
    buf prologue_buf;
    buf fn_buf;
} emit_context_t;

void emit_init(void);
void emit_reset_fn(emit_context_t *context);
void emit_finalize_fnbuf(emit_context_t *context, FILE *out);
void emit_text(FILE *out);
void emit_cstr(FILE *out);

bool emit_need_escaping(void);

typedef enum load_store {
    LOAD, STORE,
} load_store_t;

// aggregates
bool emit_eightbyte_struct(reg_t dst, const dtype_t *dtype, const dyn_agg_member *args, int *index, size_t *size, size_t limit);
void emit_store_eightbytes(reg_t base, i64 offset, reg_t lo, bool lo_written, reg_t hi, bool hi_written, bool has_hi);
void emit_store_packed(reg_t base, i64 offset, reg_t src, size_t nbytes);
void emit_zerofill(reg_t dst, i64 offset, const dtype_t *type);

void emit_make_array(reg_t dst, type_t *type, u32 len, dyn_regable *args);
void emit_store_array(reg_t dst, i64 offset, type_t *type, u32 len, dyn_regable *args);
void emit_array_access(reg_t dst, reg_t src, reg_t offset, load_store_t is_store);
void emit_elem_addr(reg_t dst, reg_t object, reg_t index);

// data processing
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
void emit_zero_out(reg_t dst);
void emit_cond_set(reg_t dst, cond_t cond);

void emit_str(reg_t dst, reg_t src, int offset);
void emit_ldr(reg_t dst, reg_t src, int offset);
void emit_str_reg(reg_t dst, reg_t src, reg_t offset);
void emit_ldr_reg(reg_t dst, reg_t src, reg_t offset);
// control flow
void emit_branch(str fn_name, str label, int index);
bool emit_branch_cond(cond_t condition, str fn_name, str label, int index);
void emit_label(str fn_name, str label, int index);
void emit_fn_prologue_epilogue(const parser_context *context);
void emit_fn_call(const str *s);
void emit_fn(str fn_name);
void emit_ret(void);

void report_error(const char *format, ...);

extern const char *text_section_header;
extern const char *string_section_header;
extern const size_t default_register_size;
