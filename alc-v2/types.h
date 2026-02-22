#pragma once

#include <stdint.h>
#include <stddef.h>

enum register_dst {
    RD_NONE, RET, PARAM, SCRATCH, NREG, STACK, FRAME
};

typedef enum {
    COND_EQ,
} cond;

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

enum sign_t {
    S_UNSIGNED, S_SIGNED
};
typedef u8 sign_t;

typedef struct {
    i32 offset;
    u32 array;
    reg_size rsize;
    register_dst reg_type : 3;
    u8 addr : 2;
    type_t *type;
} reg_t;


#define unreachable (printf("unreachable %s:%s:%d\n", __FILE__, __func__, __LINE__), __builtin_unreachable())
#define malloc_failed() (printf("malloc failed %s:%s:%d\n", __FILE__, __func__, __LINE__), abort())

#define ALIGN_TO(expr, align) (((expr) + (align) - 1) & ~((align) - 1))
#define MAX_REG_SIZE 16

#define CSI_RED "\x1b[31m"
#define CSI_GREEN "\x1b[32m"
#define CSI_RESET "\x1b[0m"
