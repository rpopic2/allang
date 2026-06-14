#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    const uint8_t *text;
    size_t text_size;
    uint32_t entry;
} bin_image;

void bin_emit(bin_image *image);
