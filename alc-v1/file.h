#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "slice.h"

long file_len(FILE *f) {
    fseek(f, 0L, SEEK_END);
    long filelen = ftell(f);
    fseek(f, 0L, SEEK_SET);
    return filelen;
}

str read_source() {
    const char *name = "todo.al";
    printf("open file %s\n", name);
    FILE *src = fopen(name, "r");
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

// typedef struct {
//     void *w;
// } writer;

typedef void *restrict writer_t;

void write_buf(writer_t *writer, const void *restrict data, size_t size) {
    memcpy(*writer, data, size);
    *writer += size;
}
void write_buf2(writer_t *writer, slice s) {
    memcpy(*writer, s.data, s.size);
    *writer += s.size;
}
void write_buf_fat(writer_t *writer, fat s) {
    size_t size = fat_size(s);
    memcpy(*writer, s.start, size);
    *writer += size;
}

