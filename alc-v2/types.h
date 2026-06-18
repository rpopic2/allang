#pragma once

#include <stdint.h>
#include <stddef.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t usize;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef ptrdiff_t isize;

#define bool _Bool

typedef u8 register_dst;
typedef u8 reg_size;

typedef struct type_t type_t;

enum register_dst {
    RD_NONE, RET, PARAM, SCRATCH, NREG, STACK, FRAME
};

typedef enum cond {
    COND_EQ, COND_NE, COND_HS, COND_LO, COND_MI, COND_PL, COND_VS, COND_VC,
    COND_HI, COND_LS, COND_GE, COND_LT, COND_GT, COND_LE, COND_AL, COND_NV
} cond_t;

#define unreachable (printf("unreachable %s:%s:%d\n", __FILE__, __func__, __LINE__), __builtin_unreachable())
#define nop (void)0
#define malloc_failed() (printf("malloc failed %s:%s:%d\n", __FILE__, __func__, __LINE__), abort())

#define ALIGN_TO(expr, align) (((expr) + (align) - 1) & ~((align) - 1))
#define MAX_REG_SIZE 16

#define CSI_RED "\x1b[31m"
#define CSI_GREEN "\x1b[32m"
#define CSI_YELLOW "\x1b[33m"
#define CSI_RESET "\x1b[0m"

