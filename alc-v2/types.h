#pragma once

#include <stdint.h>

typedef enum {
    RET, PARAM, SCRATCH, NREG, STACK, SP
} register_dst;

typedef struct {
    register_dst type;
    int offset;
} entry;

typedef int32_t i32;
typedef int64_t i64;
typedef uint32_t u32;
typedef uint64_t u64;

#define bool _Bool
#define unreachable abort()

