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

typedef u16 typeid_t;
typedef u8 register_dst;
typedef u8 reg_size;

typedef struct _type_t type_t;

typedef struct {
    register_dst type;
    reg_size size;
    typeid_t typeid;
    int offset;
    bool sign;
} reg_t;


#define unreachable (printf("unreachable %s:%s:%d\n", __FILE__, __func__, __LINE__), abort())
#define malloc_failed() (printf("malloc failed %s:%s:%d\n", __FILE__, __func__, __LINE__), abort())

#define ALIGN_TO(expr, align) ((expr) + align - 1) & ~(align - 1);
#define MAX_REG_SIZE 16
