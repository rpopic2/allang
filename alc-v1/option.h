#pragma once

#include "typedefs.h"
#include <stdbool.h>
#include <stdio.h>
typedef struct {
    bool has_value;
    u32 value;
} option_u32;

void option_u32_assign(option_u32 *self, u32 value) {
    self->value = value;
    self->has_value = true;
}

u32 option_u32_consume(option_u32 *self) {
    if (!self->has_value)
        fprintf(stderr, "! ub: consume did not have value\n");
    self->has_value = false;
    return self->value;
}

