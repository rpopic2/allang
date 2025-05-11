#pragma once

#include "error.h"
#include "list.h"
#include "stack_context.h"
#include "typedefs.h"
#include <stdbool.h>
#include <stdio.h>

typedef i32 i19;
typedef i16 i12;
typedef u8 u6;
typedef u8 u5;
typedef u8 u3;
typedef u8 u1;

#define SP 31
enum strldr_t {
    load_t, store_t
};

typedef enum {
    W, X
} sf_t;

// branches
#define RET 0xd65f03c0
#define BL 0x94000000
#define B 0x14000000
#define B_COND 0x54000000
#define CBZ 0x34000000

typedef enum {
    COND_EQ = 0b0000,
    COND_NE = 0b0001,
    COND_CS = 0b0010,
    COND_HS = 0b0010,
    COND_CC = 0b0011,
    COND_LO = 0b0011,
    COND_MI = 0b0100,
    COND_PL = 0b0101,
    COND_VS = 0b0110,
    COND_VC = 0b0111,
    COND_HI = 0b1000,
    COND_LS = 0b1001,
    COND_GE = 0b1010,
    COND_LT = 0b1011,
    COND_GT = 0b1100,
    COND_LE = 0b1101,
    COND_AL = 0b1110,
    COND_NV = 0b1111,
} cflags;

cflags cflags_flip(cflags f) {
    if (f == COND_EQ)
        return COND_NE;
    else if (f == COND_NE)
        return COND_EQ;
    else
        CompileErr("invalid flip value of %d", f);
    return COND_NV;
}

static inline u32 b_cond(i19 pcrel, cflags cond) {
    return B_COND | (pcrel >> 2) << 5 | cond;
}

static inline u32 cbz(sf_t sf, u5 reg, i19 pcrel) { 
    return CBZ | sf << 31 | (pcrel >> 2) << 5 | reg;
}

static inline u32 cbnz(sf_t sf, u5 reg, i19 pcrel) { 
    return cbz(sf, reg, pcrel) | 1 << 24;
}

// load and stores

#define STP 0x29000000
#define LDP 0x29400000
#define STR_REG 0xb8200800
#define STR_IMM 0xb9000000

typedef enum {
    E_UXTW = 0b010,
    E_LSL = 0b011,
    E_SXTW = 0b110,
    E_SXTX = 0b111,
} str_ext;

static inline u32 ldpstp(sf_t sf, bool load, u8 reg1, u8 reg2, u8 base, i8 offset_i7) {
    i8 off = sf == X ? (offset_i7 / 8) : (offset_i7 / 4);
    printf("ldp off: %x", off);
    return STP | sf << 31 | load << 22 | off << 15 | reg2 << 10 | base << 5 | reg1;
}

static inline u32 stp_pre(sf_t sf, u8 reg1, u8 reg2, u8 base, int offset_i7) {
    return ldpstp(sf, false, reg1, reg2, base, offset_i7);
}

static inline u32 ldp_post(sf_t sf, u8 reg1, u8 reg2, u8 base, int offset_i7) {
    return ldpstp(sf, true, reg1, reg2, base, offset_i7);
}

static inline u32 str_reg_f(sf_t width, u5 rt, u5 rn, u5 rm, str_ext ext, u1 amount) {
    return STR_REG | width << 30 | rm << 16 | ext << 13 | amount << 12 | rn << 5 | rt;
}

static inline u32 str_reg(sf_t width, u5 rt, u5 rn, u5 rm) {
    return str_reg_f(width, rt, rn, rm, E_LSL, 0);
}

static inline u32 str_imm(sf_t width, u5 rt, u5 rn, i12 imm) {
    if (width == W)  {
        imm /= 4;
    } else {
        imm /= 8;
    }

    return STR_IMM | width << 30 | imm << 10 | rn << 5 | rt;
}


static inline uint32_t strorldr(enum strldr_t store, u8 reg, u8 reg2, bool unscaled, int size, int offset) {
    uint32_t op = 0x38000000;
    if (!store) {
        op |= 1 << 22;
    }
    op |= reg;
    if (!unscaled)
        op |= 1 << 24;
    if (size == 8)
        op |= 0b11 << 30;
    else if (size == 1)
        ;// nop
    else if (size == 4)
        op |= 0b10 << 30;
    else if (size != 4)
        CompileErr("!!!unimpl size %d!!!", size);
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

static inline u32 ldr_size(u8 reg, u8 reg2, u16 offset, size_t size) {
    return strorldr(load_t, reg, reg2, false, size, offset);
}

void make_prelude(stack_context *s, u8 r1, u8 r2) {
    printf("make prelude %d, %d", r1, r2);
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
#define SUBS_IMM 0x7100001f
#define ADDS_IMM 0x3100001f

static inline u32 cmn(sf_t sf, u8 reg, i12 literal) {
    return ADDS_IMM | sf << 31 | literal << 10 | reg << 5;
}

static inline u32 cmp(sf_t sf, u8 reg, i12 literal, bool negate) {
    return ADDS_IMM | !negate << 30 | sf << 31 | literal << 10 | reg << 5;
}


// data processing - reg
#define MOV 0x52800000
#define ORR 0x2a000000
#define MOVK 0xf2800000
#define ADD_EXT 0x0b200000
#define ADD_SHFT 0x0b000000
#define SUB_SHFT 0x4b000000
// 0b010000 
typedef enum {
    ASH_LSL = 0b00,
    ASH_LSR = 0b01,
    ASH_ASR = 0b10,
    ASH_REVERSED = 0b11,
} add_shift;

typedef enum {
    AE_UXTB = 0b000,
    AE_UXTH = 0b001,
    AE_UXTW = 0b010,
    AE_LSL = 0b011,
    AE_UXTX = 0b011,
    AE_SXTB = 0b100,
    AE_SXTH = 0b101,
    AE_SXTW = 0b110,
    AE_SXTX = 0b111,
} add_extend;

typedef enum {
    ADDSUB_ADD = 0, ADDSUB_SUB = 1, ADDSUB_NONE = -1
} addsub;

static inline u32 add_ext_f(sf_t sf, u8 rd, u8 rn, u8 rm, add_extend option, u3 amount) {
    return ADD_EXT | sf << 31 | rm << 16 | option << 13 | amount << 10 | rn << 5 | rd;
}
static inline u32 add_shft_f(sf_t sf, addsub is_sub, u8 rd, u8 rn, u8 rm, add_shift shift, u6 amount) {
    return ADD_SHFT | sf << 31 | is_sub << 30 | shift << 22 | rm << 16 | amount << 10 | rn << 5 | rd;
}
static inline u32 add_reg_ext(sf_t sf, u8 rd, u8 rn, u8 rm) {
    return add_ext_f(sf, rd, rn, rm, AE_LSL, 0);
}
static inline u32 add_reg_shft(sf_t sf, u8 rd, u8 rn, u8 rm) {
    return add_shft_f(sf, ADDSUB_ADD, rd, rn, rm, ASH_LSL, 0);
}
static inline u32 sub_reg_shft(sf_t sf, u8 rd, u8 rn, u8 rm) {
    return add_shft_f(sf, ADDSUB_SUB, rd, rn, rm, ASH_LSL, 0);
}


static inline u32 mov(sf_t sf, u8 reg, u16 literal) {
    return MOV | sf << 31 | literal << 5 | reg;
}

static inline u32 mov_reg(sf_t sf, u8 rd, u8 rm) {
    return ORR | sf << 31 | rm << 16 | 0b11111 << 5 | rd;
}

static inline u32 movk(u8 reg, u16 literal) {
    return MOVK | 1 << 21 | literal >> 16 << 5 | reg;
}

static inline u32 sub(u8 reg1, u8 reg2, u16 literal) {
    return SUB | (literal << 10) | reg2 << 5 | reg1;
}

static inline u32 adrp(u8 reg) {
    return ADRP | reg;
}

static inline u32 add_imm(sf_t sf, u8 reg1, u8 reg2, u16 imm12) {
    return ADD_IMM | sf << 31 | imm12 << 10 | reg2 << 5 | reg1;
}

// other utils
//

sf_t nreg_sf(nreg *n) {
    sf_t op_size = W;
    u8 size = n->size;
    if (size <= 32) {
        op_size = W;
    } else if (size <= 64) { 
        op_size = X;
    } else {
        CompileErr("unknown reg size %d\n", size);
    }
    return op_size;
}
