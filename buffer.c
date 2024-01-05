#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFER_SIZE 1024

static char *buffer;
static char *buffer_head;
static char *buffer_end;

size_t buffer_len() {
    return buffer_head - buffer;
}

void buffer_init() {
    buffer = malloc(BUFER_SIZE); // TODO: use dynamic sizing buffer
    buffer_head = buffer;
    buffer_end = buffer + BUFER_SIZE;
}

void buffer_write(const char *src) {
    unsigned long len = strlen(src);
    memcpy(buffer_head, src, len);
    buffer_head += len;
    if (buffer_head > buffer_end) {
        printf("buffer overflow!");
        abort();
    }
}

int buffer_write_to_file(const char *path) {
    FILE *object_file = fopen(path, "w");
    if (!object_file) {
        printf("failed to create object file");
        return -1;
    }
    fwrite(buffer, sizeof(char), buffer_len(), object_file);
    fclose(object_file);
    return 0;
}

void buffer_putc(char c) {
    *buffer_head++ = c;
    if (buffer_head > buffer_end) {
        printf("buffer overflow!");
        abort();
    }
}

