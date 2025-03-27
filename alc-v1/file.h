#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "slice.h"

long file_len(FILE *f) {
    fseek(f, 0L, SEEK_END);
    long filelen = ftell(f);
    fseek(f, 0L, SEEK_SET);
    return filelen;
}

str read_source() {
    FILE *src = fopen("main.al", "r");
    if (src == NULL)
        exit(1);

    long src_len = file_len(src);

    char *src_buf = malloc(src_len);
    if (src_buf == NULL)
        exit(2);

    fread(src_buf, sizeof (char), src_len, src);
    fclose(src);

    str tmp = { .data = src_buf, .len = src_len };
    return tmp;
}

