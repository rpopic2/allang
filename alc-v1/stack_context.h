#pragma once

#include "list.h"
#include "str.h"

typedef enum : u8 {
    ptype_not_addr, ptype_addr_addr, ptype_stack_addr
} ptype;

typedef struct {
    str name;
    u8 reg;
    u8 bsize;
    ptype is_addr;
} nreg; // named registers
ls (nreg);

typedef struct {
    str name;
    u32 offset;
    u8 size; // size in bits
    ptype is_addr;
} obj; // named registers
ls (obj);

typedef struct {
    ls_u32 code;
    ls_u32 prologue;
    ls_u32 epilogue;
    ls_nreg named_regs;
    ls_obj objects;
    size_t stack_size;
    ls_u32 relocents;
    u8 regs_to_save[0x10];
    int regs_to_save_size;
    usize spaces_left;
    ls_u32 jump_pair_start;
    ls_nreg scratch_aliases;
} stack_context;


void stack_context_new(stack_context *s) {
    ls_new_u32(&s->code, 1024, "stack code");
    ls_new_u32(&s->prologue, 48, "stack prologue");
    ls_new_u32(&s->epilogue, 48, "stack epilogue");
    ls_new_nreg(&s->named_regs, 16, "stack named registers");
    ls_new_obj(&s->objects, 48, "objects on the stack");
    ls_new_u32(&s->relocents, 48, "relocents to push by prelude");
    s->stack_size = 0;
    s->regs_to_save_size = 0;
    s->spaces_left = 0;
    ls_new_u32(&s->jump_pair_start, 16, "jump pairs");
    ls_new_nreg(&s->scratch_aliases, 16, "scratch register aliases (&)");
}
void stack_context_free(stack_context *s) {
    ls_delete_u32(&s->code);
    ls_delete_u32(&s->prologue);
    ls_delete_u32(&s->epilogue);
    ls_delete_nreg(&s->named_regs);
    ls_delete_obj(&s->objects);
    ls_delete_u32(&s->relocents);
    ls_delete_u32(&s->jump_pair_start);
    ls_delete_nreg(&s->scratch_aliases);
}

