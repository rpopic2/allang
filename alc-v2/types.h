#pragma once

#include <stdint.h>

typedef enum {
    RD_NONE, RET, PARAM, SCRATCH, NREG, STACK, FRAME
} register_dst;

typedef enum {
    COND_EQ,
} cond;

typedef struct {
    register_dst type;
    int offset;
} reg_t;

typedef uint8_t u8;
typedef int8_t i8;
typedef int32_t i32;
typedef int64_t i64;
typedef uint32_t u32;
typedef uint64_t u64;

#define bool _Bool
#define unreachable (printf("unreachable %s:%s:%d\n", __FILE__, __func__, __LINE__), abort())

