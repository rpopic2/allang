#pragma once

#include "list.h"
#include "stack_context.h"
#include "typedefs.h"
#include <stdbool.h>
#include <stdio.h>

typedef i32 i19;
typedef u8 u5;

#define SP 31
enum strldr_t {
    load_t, store_t
};

enum sf_t {
    W, X
};

// branches
#define RET 0xd65f03c0
#define BL 0x94000000
#define CBZ 0x34000000

static inline u32 cbz(enum sf_t sf, u5 reg, i19 pcrel) { 
    return CBZ | sf << 31 | pcrel << 5 | reg;
}

static inline u32 cbnz(enum sf_t sf, u5 reg, i19 pcrel) { 
    return cbz(sf, reg, pcrel) | 1 << 24;
}

// load and stores

#define STP 0x29000000
#define LDP 0x29400000


static inline u32 ldpstp(enum sf_t sf, bool load, u8 reg1, u8 reg2, u8 base, i8 offset_i7) {
    i8 off = sf == X ? offset_i7 / 8 : offset_i7 / 4;
    return STP | sf << 31 | load << 22 | off << 14 | reg2 << 10 | base << 5 | reg1;
}

static inline u32 stp_pre(enum sf_t sf, u8 reg1, u8 reg2, u8 base, i8 offset_i7) {
    return ldpstp(sf, false, reg1, reg2, base, offset_i7);
}

static inline u32 ldp_post(enum sf_t sf, u8 reg1, u8 reg2, u8 base, i8 offset_i7) {
    return ldpstp(sf, true, reg1, reg2, base, offset_i7);
}


uint32_t strorldr(enum strldr_t store, u8 reg, u8 reg2, bool unscaled, int size, int offset) {
    uint32_t op = 0xb8000000;
    if (!store) {
        op |= 1 << 22;
    }
    op |= reg;
    if (!unscaled)
        op |= 1 << 24;
    if (size == 8)
        op |= 1 << 30;
    else if (size != 4)
        printf("!!!unimpl size %d!!!", size);
    if (unscaled) {
        op |= offset << 12;
    } else {
        op |= ((offset / size) << 10);
    }
    op |= reg2 << 5;
    return op;
}

static inline u32 store(u8 reg, u8 reg2, u16 offset) {
    return strorldr(store_t, reg, reg2, false, sizeof (u64), offset);
}

static inline u32 ldr(u8 reg, u8 reg2, u16 offset) {
    return strorldr(load_t, reg, reg2, false, sizeof (u64), offset);
}

void make_prelude(stack_context *s, u8 r1, u8 r2) {
    u32 op = stp_pre(X, r1, r2, SP, s->stack_size);
    ls_add_u32(&s->prologue, op);
    op = ldp_post(X, r1, r2, SP, s->stack_size);
    ls_add_u32(&s->epilogue, op);
    s->stack_size += 0x10;
}

// data processing - imm
#define ADRP 0x90000000
#define ADD_IMM 0x91000000
#define SUB 0xd1000000

// data processing - reg
#define MOV 0x52800000
#define ORR 0x2a000000
#define MOVK 0xf2800000
#define ADD_EXT 0x0b000000

static inline u32 add_ext(enum sf_t sf, u8 rd, u8 rn, u8 rm) {
    return ADD_EXT | sf << 31 | 1 << 21 | rm << 16 | rn << 5 | rd;
}
static inline u32 add_shft(enum sf_t sf, u8 rd, u8 rn, u8 rm) {
    return ADD_EXT | sf << 31 | rm << 16 | rn << 5 | rd;
}


static inline u32 mov(u8 reg, u16 literal) {
    return MOV | literal << 5 | reg;
}

static inline u32 mov_reg(enum sf_t sf, u8 rd, u8 rm) {
    return ORR | sf << 31 | rm << 16 | 0b11111 << 5 | rd;
}

static inline u32 movk(u8 reg, u16 literal) {
    return MOVK | 1 << 21 | literal >> 16 << 5 | reg;
}

static inline u32 sub(u8 reg1, u8 reg2, u16 literal) {
    return SUB | (literal << 10) | reg2 << 5 | reg1;
}

static inline u32 add(enum sf_t sf, u8 reg1, u8 reg2, u16 value) {
    return ADD_IMM | sf << 31 | (value << 0xa) | (reg2 << 5) | (reg1);
}

static inline u32 adrp(u8 reg) {
    return ADRP | reg;
}

static inline u32 add_imm(enum sf_t sf, u8 reg1, u8 reg2, u16 imm12) {
    return ADD_IMM | sf << 31 | imm12 << 10 | reg2 << 5 | reg1;
}
