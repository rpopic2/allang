#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    const char *name;
} bin_import;

typedef struct {
    uint32_t site;
    uint32_t import;
} bin_extcall;

typedef struct {
    const uint8_t *text;
    size_t text_size;
    uint32_t entry;
    const bin_import *imports;
    uint32_t imports_count;
    const bin_extcall *extcalls;
    uint32_t extcalls_count;
} bin_image;

void bin_emit(bin_image *image);
