#pragma once

#include <stdio.h>
#include <stdbool.h>

typedef struct str str;

typedef enum {
    RET, PARAM, SCRATCH
} register_dst;

void emit_init(void);
void emit(FILE *out);

bool emit_need_escaping(void);

void emit_mov(register_dst reg_dst, int regidx, int value);
void emit_add(register_dst reg_dst, int regidx, int value);
void emit_string_lit(register_dst reg_dst, int regidx, const str *s);

void emit_fn_prologue(void);
void emit_fn_epilogue(void);
void emit_fn_call(const str *s);
void emit_mainfn(void);
void emit_ret(void);

