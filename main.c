#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.c"

#define MAX_TOKEN_LEN 64

const char *header = ".global _main\n";
const char *ins_mov = "mov w";
const char *ins_add = "add w";
const char *ins_ret = "ret";

enum TokenType {
    None, Fn, Number
};

int main(int argc, const char **argv) {
    if (argc != 2) {
        printf("please provide source file.\n");
        return 1;
    }
    const char *source_path = argv[1];
    const unsigned long source_path_len = strlen(source_path);

    FILE *source_file = fopen(source_path, "r");
    if (!source_file) {
        printf("source file does not exist\n");
        return 2;
    }

    buffer_init();

    buffer_write(header);

    enum TokenType next_token = None;
    char *token = malloc(MAX_TOKEN_LEN);
    size_t token_buf_idx = 0;
    char reg = '0';

    int c;
    while (1) {
        int c = fgetc(source_file);
        int is_newline = c == '\n';
        if (c != ' ' && c != ',' && !is_newline && c != EOF) {
            token[token_buf_idx++] = c;
            continue;
        }
        if (token_buf_idx == 0) {
            if (c == EOF)
                break;
            continue;
        }

        token[token_buf_idx++] = '\0';
        printf("[%s]\n", token);
        switch (next_token) {
        case None:
            if (token[0] >= '0' && token[0] <= '9') {
                buffer_write(ins_mov);
                buffer_putc(reg++);
                buffer_write(", #");
                buffer_write(token);
                buffer_putc('\n');
            } else if (token[0] == '+') {
                buffer_write(ins_add);
                buffer_putc(reg);
                buffer_write(", w0, w1\n");
            } else if (strcmp(token, "fn") == 0) {
                next_token = Fn;
            } else if (strcmp(token, "ret") == 0) {
                buffer_write(ins_ret);
                buffer_putc('\n');
            }
            break;
        case Fn:
            buffer_putc('_');
            buffer_write(token);
            buffer_putc('\n');
            next_token = None;
            break;
        case Number:
            break;
        }
        if (c == EOF)
            break;
        token_buf_idx = 0;
        if (is_newline)
            reg = '0';
    }
    char *object_path = malloc(source_path_len);
    strcpy(object_path, source_path);
    object_path[source_path_len - 2] = 's';
    object_path[source_path_len - 1] = '\0';
    buffer_write_to_file("a.s");
    fclose(source_file);
    free(buffer);
    free(token);
    return 0;
}

