#pragma once

#include "list.h"
#include "str.h"

typedef enum : u8 {
    not_addr, addr_addr, stack_addr
} ptype;

typedef struct {
    str name;
    u8 reg;
    u8 size;
    ptype is_addr;
} nreg; // named registers
ls (nreg);

typedef struct {
    ls_u32 code;
    ls_u32 prologue;
    ls_u32 epilogue;
    ls_nreg named_regs;
    size_t stack_size;
    ls_u32 relocents;
    u8 regs_to_save[0x10];
    int regs_to_save_size;
} stack_context;


void stack_context_new(stack_context *s) {
    ls_new_u32(&s->code, 1024, "stack code");
    ls_new_u32(&s->prologue, 48, "stack prologue");
    ls_new_u32(&s->epilogue, 48, "stack epilogue");
    ls_new_nreg(&s->named_regs, 16, "stack named registers");
    ls_new_u32(&s->relocents, 48, "relocents to push by prelude");
    s->stack_size = 0;
    s->regs_to_save_size = 0;
}
void stack_context_free(stack_context *s) {
    ls_delete_u32(&s->code);
    ls_delete_u32(&s->prologue);
    ls_delete_u32(&s->epilogue);
    ls_delete_nreg(&s->named_regs);
    ls_delete_u32(&s->relocents);
}

