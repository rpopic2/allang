#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "err.h"
#include "str.h"
#include "emit.h"
#include "opt.h"
#include "hashset.h"

OPT_GENERIC(i64)

int lineno = 1;
bool has_compile_err = false;

str cur_token = (str){.data = 0, .end = 0};
void lex(iter *src) {
retry:
    cur_token = (str){.data = src->cur};
    while (true) {
        char c = *src->cur;
        if (c == '"') {
            do {
                c = *(++src->cur);
            } while (c != '"' && c != '\n');
            cur_token.end = ++src->cur;
            break;
        }
        if (c == '/' && src->cur[1] == '/') {
            do {
                c = *(++src->cur);
            } while (c != '\n');
            cur_token.data = src->cur;
        }
        if (src->cur[0] == '\n')
            ++lineno;
        if (c == ',' || c == '\n' || c == ' ' || c == '\0') {
            cur_token.end = src->cur++;
            if (c == ',')
                src->cur++;
            break;
        }
        ++src->cur;
    }
    if (cur_token.end > src->end) {
        cur_token = str_null;
        return;
    }
    if (str_len(&cur_token) == 0)
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


void literal_string(const parser_context *restrict state, const str *restrict token) {
    bool escape = emit_need_escaping();
    if (token->end[-1] != '"') {
        compile_err("expected closing \"\n");
    }
    if (!escape) {
        emit_string_lit(state->reg.type, state->reg.offset, token);
        return;
    }
    size_t len = str_len(token);
    iter unescaped = iter_init(malloc(len), len);

    for (size_t i = 0; i < len; ++i) {
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
                i64 number = strtoll(token->data, NULL, 0);
                if (number > (signed)sizeof (char)) {
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
    emit_string_lit(state->reg.type, state->reg.offset, &unescaped_s);
    free(unescaped.start);
}

opt_i64 lit_numeric(const str *token) {
    i64 value = 0;
    if (isdigit(token->data[0])) {
        value = strtoll(token->data, NULL, 0);
    } else if (token->data[0] == '\'') {
        char c = token->data[1];
	if (token->end[-1] != '\'') {
	    compile_err("expected closing \'\n");
	}
	value = c;
    } else if (str_eq_lit(token, "true")) {
        value = 1;
    } else if (str_eq_lit(token, "false")) {
        value = 0;
    } else {
        return opt_long_none;
    }
    return opt_i64_some(value);
}

typedef struct {
    enum {
        NONE, VALUE, REG
    } tag;
    union {
        i64 value;
        reg_t reg;
    };
} regable;
// static const entry SP = (entry){ .type = STACK };
static const reg_t FP = (reg_t){ .type = FRAME };

regable read_regable(const str *token) {
    regable result = (regable){ .value = 0, .tag = 0};
    if (isupper(token->data[0])) {
        reg_t *e = find_id(token);
        if (e->type == RD_NONE) {
            compile_err("unknown id "), str_fprint(token, stderr);
        } else {
            result.tag = REG;
            result.reg = *e;
        }
    } else {
        if_opt(i64, value, = lit_numeric(token)) {
            result.tag = VALUE;
            result.value = value;
        }
    }
    return result;
}

void binary_op(const regable *restrict lhs, parser_context *restrict state) {
    lex(&state->src);
    str op_token = cur_token;

    lex(&state->src);
    str operand_token = cur_token;
    regable rhs = read_regable(&operand_token);

    if (rhs.tag == NONE) {
        compile_err("expected operand\n");
    }
    if (op_token.data[0] == '+') {
        if (lhs->tag == VALUE && rhs.tag == VALUE) {
            emit_mov(state->reg.type, state->reg.offset, lhs->value + rhs.value);
        } else if (lhs->tag == VALUE && rhs.tag == REG) {
            emit_add(state->reg, rhs.reg, lhs->value);
        } else if(lhs->tag == REG && rhs.tag == VALUE) {
            emit_add(state->reg, lhs->reg, rhs.value);
        } else if (lhs->tag == REG && rhs.tag == REG) {
            emit_add_reg(state->reg, lhs->reg, rhs.reg);
        } else unreachable;
    } else if (op_token.data[0] == '-') {
        if (lhs->tag == VALUE && rhs.tag == VALUE) {
            emit_mov(state->reg.type, state->reg.offset, lhs->value - rhs.value);
        } else if (lhs->tag == VALUE && rhs.tag == REG) {
            int tmp_reg_off = state->reg.offset;
            if (state->reg.type == SCRATCH) {
                tmp_reg_off += 1;
            }
            emit_mov(SCRATCH, tmp_reg_off, lhs->value);
            emit_sub_reg(state->reg, (reg_t){SCRATCH, tmp_reg_off}, rhs.reg);
        } else if(lhs->tag == REG && rhs.tag == VALUE) {
            emit_sub(state->reg, lhs->reg, rhs.value);
        } else if (lhs->tag == REG && rhs.tag == REG) {
            emit_sub_reg(state->reg, lhs->reg, rhs.reg);
        } else unreachable;
    } else {
        compile_err("unknown operator\n");
    }
}

bool expr(const str *restrict token, parser_context *restrict state) {
    if (token->data[0] == '"') {
        literal_string(state, token);
        return true;
    }
    if (token->data[0] == '[') {
        str id = {.data = token->data + 1, .end = token->end - 1};
        reg_t *e = find_id(&id);
        if (token->end[-1] != ']') {
            compile_err("closing ']' expected\n");
        }
        if (e->type != NONE)
            emit_ldr_fp(state->reg, e->offset);
        return true;
    }

    regable lhs = read_regable(token);
    if (lhs.tag == NONE) {
        return false;
    }

    char next = token->end[1];
    if (next == '#' || next == '+') {
        binary_op(&lhs, state);
    } else {
        if (lhs.tag == VALUE) {
            emit_mov(state->reg.type, state->reg.offset, lhs.value);
        } else if (lhs.tag == REG) {
            const reg_t *nreg = &lhs.reg;
            if (nreg->type == NREG) {
                emit_mov_reg(state->reg.type, state->reg.offset, nreg->type, nreg->offset);
            } else if (nreg->type == STACK) {
                emit_sub(state->reg, FP, nreg->offset);
            } else {
                unreachable;
            }
        }
    }
    return true;
}

bool expr_line(str in_token, parser_context *state) {
    str *token = &in_token;
    bool ok = expr(token, state);
    if (!ok)
        return false;

    while (cur_token.end[0] == ',' && isspace(cur_token.end[1])) {
        printd(", ");
        state->reg.offset++;
        lex(&state->src);
        *token = cur_token;
        str_printd(token);
        ok = expr(token, state);
        if (!ok)
            break;
    }
    state->reg.offset = 0;
    state->reg.type = SCRATCH;
    return true;
}

bool stmt(const str *restrict token, parser_context *restrict state) {
    if (isupper(token->data[0])) {
        if (streq(token->end, " ::")) {
            lex(&state->src);

            if (cur_token.end[0] != '\n') {
                state->reg.type = NREG;
            }
            state->reg.offset = state->nreg_count++;
            state->target = add_id(*token, NREG, state->reg.offset);
            return true;
        }
    } else if (token->data[0] == '[') {
        str id = {.data = token->data + 1, .end = token->end - 1};
        reg_t *e = find_id(&id);
        if (token->end[-1] != ']') {
            compile_err("closing ']' expected\n");
        }
        if (e->type != NONE)
            return false;
        state->reg.type = SCRATCH;
        state->stack_size += sizeof (i32);
        int offset = state->stack_size;
        state->target = add_id(id, STACK, offset);

        lex(&state->src);
        return true;
    }
    return false;
}

void parse(const str *restrict token, parser_context *restrict state) {
    if (stmt(token, state)) {

    } else if (expr_line(*token, state)) {

    } else if (str_eq_lit(token, "=[]")) {
        reg_t src = (reg_t){.type = state->reg.type, .offset = state->reg.offset};
        emit_str_fp(src, state->target->offset);
    } else if (str_eq_lit(token, "=")) {
        state->reg = *state->target;
    } else if (str_eq_lit(token, "ret")) {
        state->reg.type = RET;
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
        state->reg.offset = 0;
        state->reg.type = SCRATCH;
        state->calls_fn = true;
    } else if (islower(token->data[0]) || token->data[0] == '_') {
        state->reg.type = PARAM;
        state->deferred_fn_call = *token;
    } else {
        compile_err("unknown token "), str_fprint(token, stderr);
    }
}


int main(int argc, const char *argv[]) {
    if (argc == 1) {
        fprintf(stderr, "usage: alc [filename]\n");
        exit(EXIT_FAILURE);
    }
    const char *source_name = argv[1];
    FILE *source_file = fopen(argv[1], "r");
    if (source_file == NULL) {
        fprintf(stderr, "error: could not open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    fseek(source_file, 0, SEEK_END);
    size_t source_len = (size_t)ftell(source_file);
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

    parser_context *state = &(parser_context){
        .reg = (reg_t) {.type = SCRATCH, .offset = 0 },
        .nreg_count = 0,
        .src = _src,
        .calls_fn = false,
        .stack_size = 0,
        .target = NULL,
    };
    iter *src = &state->src;

    while (src->cur < src->end) {
        lex(src);
        if (str_len(&cur_token) == 0)
            continue;
        str_printd(&cur_token);
        parse(&cur_token, state);
    }

    emit_fn_prologue_epilogue(state);
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

