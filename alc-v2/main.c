#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "emit.h"

int lineno = 1;

void lex(str *token, iter *src) {
    while (true) {
        char c = *src->cur;
        if (c == '"') {
            do {
                c = *(++src->cur);
            } while (c != '"' && c != '\n');
            token->end = ++src->cur;
            ++src->cur;
            break;
        }
        if (c == '\n')
            ++lineno;
        if (c == ',' || c == '\n' || c == ' ' || c == '\0') {
            token->end = src->cur++;
            if (c == ',')
                src->cur++;
            break;
        }
        ++src->cur;
    }
}

void compile_err(const char *format, ...) {
    fprintf(stderr, "line %d: ", lineno);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

typedef struct {
    enum {
        RET, PARAM, SCRATCH
    } reg_dst;
    int reg_off;
} parser_context;

void literal_numeric(const parser_context *state, long number) {
    if (state->reg_dst == RET)
        emit_mov_retreg(state->reg_off, number);
    else if (state->reg_dst == SCRATCH)
        emit_mov_scratch(state->reg_off, number);
    else if (state->reg_dst == PARAM)
        emit_mov_param(state->reg_off, number);
    else
        fputs("unknown destinination register state\n", stderr);
}

void literal_string(const parser_context *state, const str *token) {
    bool escape = emit_need_escaping();
    if (token->end[-1] != '"') {
        compile_err("expected closing \"\n");
    }
    if (!escape) {
        emit_string_lit(state->reg_off, token);
        return;
    }
    size_t len = str_len(token);
    iter unescaped = iter_init(malloc(len), len);

    for (int i = 0; i < len; ++i) {
        char c = token->data[i];
        if (c != '\\')
            *unescaped.cur++ = c;
        else {
            c = token->data[++i];
            char result = ' ';
            switch (c) {
            case 'n':
                result = '\n';
                break;
            case 't':
                result = '\t';
                break;
            case '0':
                result = '\0';
                break;
            case '\\':
                result = '\\';
                break;
            default:;
                long number = strtol(token->data, NULL, 0);
                if (number > sizeof (char)) {
                    compile_err("%d is too large for a string literal", number);
                } else {
                    result = (char)number;
                }
                break;
            }
            *unescaped.cur++ = result;
        }
    }

    str unescaped_s = str_from_iter(&unescaped);
    emit_string_lit(state->reg_off, &unescaped_s);
}

void parse(const str *token, parser_context *state) {
    if (is_digit(token->data[0])) {
        long number = strtol(token->data, NULL, 0);
        literal_numeric(state, number);
    } else if (token->data[0] == '\'') {
        char c = token->data[1];
        literal_numeric(state, c);
        if (token->end[-1] != '\'') {
            compile_err("expected closing \'\n");
        }
    } else if (token->data[0] == '"') {
        literal_string(state, token);
    } else if (str_eq_lit(token, "ret")) {
        state->reg_dst = RET;
    } else if (memcmp(token->end - 2, "=>", 2) == 0) {
        emit_fn_call(&(str){token->data, token->end - 2});
    } else {
        compile_err("unknown token "), str_fprint(token, stderr);
    }

    char end = token->end[-1];
    if (end == ',') {
        ++state->reg_off;
    } else if (token->end[0] == '\n') {
        state->reg_off = 0;
    }
}


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

    iter _src = { .start = source_start, .cur = source_start, .end = source_start + source_len };
    iter *src = &_src;

    emit_init();

    emit_mainfn();
    emit_fn_prologue();

    parser_context _state;
    parser_context *state = &_state;
    state->reg_dst = SCRATCH;
    state->reg_off = 0;

    while (src->cur < src->end) {
        str _token = {.data = src->cur};
        str *token = &_token;
        lex(token, src);
        str_print(token);
        parse(token, state);
    }

    emit_fn_epilogue();
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

