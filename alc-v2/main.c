#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "emit.h"

int lineno = 1;
bool has_compile_err = false;

void lex(str *token, iter *src) {
    while (true) {
        char c = *src->cur;
        if (c == '"') {
            do {
                c = *(++src->cur);
            } while (c != '"' && c != '\n');
            token->end = ++src->cur;
            break;
        }
        if (c == '/' && src->cur[1] == '/') {
            do {
                c = *(++src->cur);
            } while (c != '\n');
            token->data = src->cur;
        }
        if (src->cur[0] == '\n')
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
    has_compile_err = true;
    fputs("\x1b[31m", stderr);
    fprintf(stderr, "line %d: ", lineno);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\x1b[0m", stderr);
}

typedef struct {
    register_dst reg_dst;
    int reg_off;
    str deferred_fn_call;
} parser_context;

void literal_string(const parser_context *state, const str *token) {
    bool escape = emit_need_escaping();
    if (token->end[-1] != '"') {
        compile_err("expected closing \"\n");
    }
    if (!escape) {
        emit_string_lit(state->reg_dst, state->reg_off, token);
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
    emit_string_lit(state->reg_dst, state->reg_off, &unescaped_s);
    free(unescaped.start);
}

void check_line_expr(const str *token, parser_context *state) {
    state->reg_off++;
    return;
    // TODO need to come up with better comma detection...
    if (state->reg_off <= 0)
        return;
    str *prev_comma = &(str){ .data = token->data - 2, .end = token->data };
    if (str_eq_lit(prev_comma, ", "))
        return;
}

void parse(const str *token, parser_context *state) {
    if (is_digit(token->data[0])) {
        long number = strtol(token->data, NULL, 0);
        emit_mov(state->reg_dst, state->reg_off, number);
        check_line_expr(token, state);
    } else if (token->data[0] == '\'') {
        char c = token->data[1];
        emit_mov(state->reg_dst, state->reg_off, c);
        if (token->end[-1] != '\'') {
            compile_err("expected closing \'\n");
        }
        check_line_expr(token, state);
    } else if (token->data[0] == '"') {
        literal_string(state, token);
        check_line_expr(token, state);
    } else if (str_eq_lit(token, "true")) {
        emit_mov(state->reg_dst, state->reg_off, 1);
    } else if (str_eq_lit(token, "false")) {
        emit_mov(state->reg_dst, state->reg_off, 0);
    } else if (str_eq_lit(token, "ret")) {
        state->reg_dst = RET;
    } else if (str_ends_with(token, "=>")) {
        str *fn_name = &(str){token->data, token->end - 2};
        if (!str_is_empty(&state->deferred_fn_call) && str_is_empty(fn_name)) {
            str s = str_move(&state->deferred_fn_call);
            emit_fn_call(&s);
        } else if (!str_is_empty(fn_name)) {
            emit_fn_call(fn_name);
        } else {
            compile_err("empty function name");
        }
        state->reg_off = 0;
        state->reg_dst = SCRATCH;
    } else if (is_lowercase(token->data[0]) || token->data[0] == '_') {
        state->reg_dst = PARAM;
        state->deferred_fn_call = *token;
    } else {
        compile_err("unknown token "), str_fprint(token, stderr);
    }

    if (token->end[0] == '\n') {
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
        if (str_len(token) == 0)
            continue;
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

    return has_compile_err;
}

