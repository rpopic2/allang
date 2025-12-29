#pragma once

#include <stdio.h>
#include <stdbool.h>

#include "str.h"
#include "types.h"

typedef struct {
    register_dst reg_dst;
    int reg_off;
    int nreg_count;
    str deferred_fn_call;
    iter src;
    bool calls_fn;
} parser_context;

void emit_init(void);
void emit(FILE *out);

bool emit_need_escaping(void);

void emit_mov(register_dst reg_dst, int regidx, i64 value);
void emit_mov_reg(register_dst reg_dst, int regidx, register_dst reg_src, int regidx_src);
void emit_add(entry dst, entry lhs, i64 rhs);
void emit_add_reg(entry dst, entry lhs, entry rhs);
void emit_string_lit(register_dst reg_dst, int regidx, const str *s);

void emit_fn_prologue_epilogue(const parser_context *context);
void emit_fn_call(const str *s);
void emit_mainfn(void);
void emit_ret(void);

extern const char *text_section_header;
extern const char *string_section_header;
