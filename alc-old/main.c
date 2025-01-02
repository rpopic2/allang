#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.c"
#include "hashmap.h"

#define MAX_TOKEN_LEN 64
#define SENTRY_DEFAULT 0
#define SENTRY_NAME 1

const char *header = ".global _main\n.p2align 2\n\n";
const char *ins_mov = "mov w";
const char *ins_add = "add w";
const char *ins_ret = "ret";
const char *ins_str = "str w";
const char *ins_add_sp = "add sp, sp, #0x";
const char *ins_sub_sp = "sub sp, sp, #0x";

enum TokenType {
    Default, Fn, Name,
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

    Buffer buf;
    buffer_init(&buf);
    buffer_puts(&buf, header);

    enum TokenType next_token = Default;
    char *token = malloc(MAX_TOKEN_LEN);
    size_t token_buf_idx = 0;

    char reg = '0';

    int stack_size = 0x00;
    char stack_size_str[4];
    char itoa_buf[8];
    const int STACK_INC = 0x10;
    int stack_current_top = stack_size;

    int next_sentry = SENTRY_DEFAULT;
    printf("stack sentry phase\n");

    struct HashMap names;
    hashmap_new(&names);

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

        if (next_sentry) {
            next_sentry = SENTRY_DEFAULT;
        } else {
            if (strcmp(token, "=>") == 0) {
                stack_current_top += 0x04;
                while (stack_current_top > stack_size) {
                    stack_size += 0x10;
                }
                next_sentry = SENTRY_NAME;
            }
        }
        if (c == EOF)
            break;
        token_buf_idx = 0;
    }

    sprintf(stack_size_str, "%x", stack_size);
    printf("stack size: %x\n", stack_size);
    stack_current_top = stack_size;
    token_buf_idx = 0;
    // token[0] = '\0';
    rewind(source_file);
    // actual compiling
    printf("compile phase\n");
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
        case Default:
            if (token[0] >= '0' && token[0] <= '9') {
                buffer_puts(&buf, ins_mov);
                buffer_putc(&buf, reg++);
                buffer_puts(&buf, ", #");
                buffer_puts(&buf, token);
                buffer_putc(&buf, '\n');
            } else if (token[0] == '+') {
                buffer_puts(&buf, ins_add);
                buffer_putc(&buf, reg);
                buffer_puts(&buf, ", w0, w1\n");
            } else if (strcmp(token, "->") == 0) {
                reg = '0';
            } else if (strcmp(token, "=>") == 0) {
                next_token = Name;
            } else if (strcmp(token, "fn") == 0) {
                next_token = Fn;
            } else if (strcmp(token, "ret") == 0) {
                buffer_puts(&buf, ins_add_sp);
                buffer_puts(&buf, stack_size_str);
                buffer_puts(&buf, "\n");
                buffer_puts(&buf, ins_ret);
                buffer_putc(&buf, '\n');
            }
            break;
        case Fn:
            buffer_putc(&buf, '_');
            buffer_puts(&buf, token);
            buffer_putc(&buf, '\n');
            buffer_puts(&buf, ins_sub_sp);
            buffer_puts(&buf, stack_size_str);
            buffer_puts(&buf, "\n");
            next_token = Default;
            break;
        case Name:;
            size_t addr = hashmap_get(&names, token);
            reg = '0';
            buffer_puts(&buf, ins_str);
            buffer_putc(&buf, reg);
            buffer_puts(&buf, ", [sp, #0x");
            if (addr) {
                sprintf(itoa_buf, "%zx", addr);
            } else {
                stack_current_top -= 0x4;
                sprintf(itoa_buf, "%x", stack_current_top);
                hashmap_add(&names, token, stack_current_top);
            }
            buffer_puts(&buf, itoa_buf);
            buffer_puts(&buf, "]\n");
            next_token = Default;
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
    buffer_putc(&buf, '\n');
    buffer_write_to_file(&buf, "a.s");
    fclose(source_file);
    buffer_free(&buf);
    hashmap_free(&names);
    free(token);
    return 0;
}

