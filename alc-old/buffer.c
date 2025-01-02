#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFER_SIZE 1024

typedef struct {
    char *buffer;
    char *buffer_head;
    char *buffer_end;
} Buffer;

void buffer_init(Buffer *buf) {
    char *ptr = malloc(BUFER_SIZE); // TODO: use dynamic sizing buffer
    buf->buffer = ptr;
    buf->buffer_head = ptr;
    buf->buffer_end = ptr + BUFER_SIZE;
}

void buffer_free(Buffer *buf) {
    free(buf->buffer);
}
size_t buffer_len(Buffer *buf) {
    return buf->buffer_head - buf->buffer;
}

char *buffer_get_head(Buffer *buf) {
    return buf->buffer_head;
}

void buffer_puts(Buffer *buf, const char *src) {
    unsigned long len = strlen(src);
    memcpy(buf->buffer_head, src, len);
    buf->buffer_head += len;
    if (buf->buffer_head > buf->buffer_end) {
        printf("buffer overflow!");
        abort();
    }
}

int buffer_write_to_file(Buffer *buf, const char *path) {
    FILE *object_file = fopen(path, "w");
    if (!object_file) {
        printf("failed to create object file");
        return -1;
    }
    fwrite(buf->buffer, sizeof(char), buffer_len(buf), object_file);
    fclose(object_file);
    return 0;
}

void buffer_putc(Buffer *buf, char c) {
    *(buf->buffer_head)++ = c;
    if (buf->buffer_head > buf->buffer_end) {
        printf("buffer overflow!");
        abort();
    }
}

