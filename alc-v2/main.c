#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "emit.h"

int main(int argc, const char *argv[]) {
    const char *source_name = argv[1];
    FILE *source_file = fopen(argv[1], "r");
    if (source_file == NULL) {
        fprintf(stderr, "error: could not open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    fseek(source_file, 0, SEEK_END);
    long source_len = ftell(source_file);
    rewind(source_file);

    char *source_start = malloc(source_len);
    if (source_start == NULL) {
        fputs("error: malloc failed\n", stderr);
        exit(EXIT_FAILURE);
    }
    unsigned long bytes_read = fread(source_start, sizeof (char), source_len, source_file);
    if (bytes_read != source_len) {
        fputs("error: failed to read file\n", stderr);
        exit(EXIT_FAILURE);
    }

    iter src = { .start = source_start, .cur = source_start, .end = source_start + source_len };

    emit_init();

    emit_mainfn();

    str token = {.data = src.cur};
    while (true) {
        char c = *src.cur;
        if (c == '\n' || c == '\0') {
            token.end = src.cur;
            *src.cur = '\0';
            break;
        }
        ++src.cur;
    }

    if (is_digit(token.data[0])) {
        long number = strtol(token.data, NULL, 0);
        emit_mov_retreg(0, number);
    }
    emit_ret();

    size_t source_name_len = strlen(source_name); 
    char *out_name = malloc(source_name_len + 1);
    strncpy(out_name, source_name, source_name_len - 1);
    out_name[source_name_len - 2] = 's';

    FILE *object_file = fopen(out_name, "w");
    if (object_file == NULL) {
        fprintf(stderr, "error: failed to create file\n");
        exit(EXIT_FAILURE);
    }
    emit(object_file);
}

