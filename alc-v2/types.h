#pragma once

#include <stdint.h>

typedef enum {
    RET, PARAM, SCRATCH, NREG
} register_dst;

typedef struct {
    register_dst type;
    int offset;
} entry;

typedef int64_t i64;
typedef uint64_t u64;

