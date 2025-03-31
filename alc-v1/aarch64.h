#pragma once

#include "typedefs.h"
#include <stdbool.h>
#include <stdio.h>
#define RET 0xd65f03c0
#define MOV 0x52800000
#define MOVK 0xf2800000
#define ADD_IMM 0x91000000
#define SUB 0xd1000000
#define SP 31

enum strldr_t {
    load_t, store_t
};

static inline u32 mov(u8 reg, u16 literal) {
    return MOV | literal << 5 | reg;
}

static inline u32 movk(u8 reg, u16 literal) {
    return MOVK | 1 << 21 | literal >> 16 << 5 | reg;
}

static inline u32 sub(u8 reg1, u8 reg2, u16 literal) {
    return SUB | (literal << 10) | reg2 << 5 | reg1;
}

static inline u32 add(u8 reg1, u8 reg2, u16 offset) {
    return ADD_IMM | (offset << 0xa) | (reg2 << 5) | (reg1);
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

