#pragma once

#include <stdio.h>
#include <stdbool.h>

void emit_init(void);
void emit(FILE *out);
void emit_mov_retreg(int regidx, int value);
void emit_mov_scratch(int regidx, int value);
void emit_mov_param(int regidx, int value);
void emit_mainfn(void);
void emit_ret(void);

