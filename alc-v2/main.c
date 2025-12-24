#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "emit.h"

int lineno = 1;
bool has_compile_err = false;

void lex(str *token, iter *src) {
retry:
    *token = (str){.data = src->cur};
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
    if (token->end > src->end) {
        *token = str_null;
        return;
    }
    if (str_len(token) == 0)
        goto retry;
}

void compile_err(const char *format, ...) {
    has_compile_err = true;
    fputs("\x1b[31m", stderr);
    fprintf(stderr, "error in line %d: ", lineno);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\x1b[0m", stderr);
}
void compile_warning(const char *format, ...) {
    fputs("\x1b[33m", stderr);
    fprintf(stderr, "warning in line %d: ", lineno);

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
    iter src;
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

bool expr_in(const str *token, parser_context *state) {
    if (isdigit(token->data[0])) {
        long number = strtol(token->data, NULL, 0);
        emit_mov(state->reg_dst, state->reg_off, number);
    } else if (token->data[0] == '\'') {
        char c = token->data[1];
        emit_mov(state->reg_dst, state->reg_off, c);
        if (token->end[-1] != '\'') {
            compile_err("expected closing \'\n");
        }
    } else if (str_eq_lit(token, "true")) {
        emit_mov(state->reg_dst, state->reg_off, 1);
    } else if (str_eq_lit(token, "false")) {
        emit_mov(state->reg_dst, state->reg_off, 0);
    } else if (token->data[0] == '"') {
        literal_string(state, token);
    } else {
        return false;
    }
    return true;
}

bool expr(str in_token, parser_context *state) {
    str *token = &in_token;
    bool ok = expr_in(token, state);
    if (!ok)
        return false;

    while (token->end[0] == ',' && isspace(token->end[1])) {
        state->reg_off++;
        lex(token, &state->src);
        str_print(token);
        ok = expr_in(token, state);
        if (!ok)
            break;
    }
    if (token->end[0] == '\n') {
        state->reg_off = 0;
    }
    return true;
}

void parse(const str *token, parser_context *state) {
    if (expr(*token, state)) {

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
    } else if (islower(token->data[0]) || token->data[0] == '_') {
        state->reg_dst = PARAM;
        state->deferred_fn_call = *token;
    } else {
        compile_err("unknown token "), str_fprint(token, stderr);
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
	memset(source_start, 0, source_len);
    if (source_start == NULL) {
        fputs("error: malloc failed\n", stderr);
        exit(EXIT_FAILURE);
    }
    unsigned long bytes_read = fread(source_start, sizeof (char), source_len, source_file);
    if (bytes_read > source_len) {
		fprintf(stderr, "error: buffer overflow. expected %ld bytes but read %lu bytes\n", source_len, bytes_read);
        exit(EXIT_FAILURE);
    }

    iter _src = { .start = source_start, .cur = source_start, .end = source_start + source_len };

    emit_init();

    emit_mainfn();
    emit_fn_prologue();

    parser_context _state;
    parser_context *state = &_state;
    state->reg_dst = SCRATCH;
    state->reg_off = 0;
    state->src = _src;
    iter *src = &state->src;

    str *token = &(str){.data = 0, .end = 0};
    while (src->cur < src->end) {
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
	memset(out_name, 0, source_name_len + 1);
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

