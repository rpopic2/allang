#include <inttypes.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "err.h"
#include "str.h"
#include "emit.h"
#include "opt.h"
#include "mini_hashset.h"
#include "hashmap.h"

OPT_GENERIC(i64)

unsigned char lineno = 1;
unsigned char indent = 0;
bool has_compile_err = false;


inline static bool is_id(char c) {
    return isalnum(c) || c == '_';
}

void lex(parser_context *context) {
retry:;
    iter *src = context->src;
    token_t *cur_token = &context->cur_token;
    *cur_token = (token_t){.data = src->cur, .end = src->cur};
    cur_token->lineno = lineno;
    cur_token->indent = indent;

    while (true) {
        if (src->cur > src->end) {
            *cur_token = (token_t){0};
            return;
        }
        char c = *src->cur;
        if (c == '"') {
            do {
                c = *(++src->cur);
            } while (c != '"' && c != '\n');
            cur_token->end = ++src->cur;
            break;
        }
        if (c == '/' && src->cur[1] == '/') {
            do {
                c = *(++src->cur);
            } while (c != '\n');
            cur_token->data = src->cur;
        }
        c = *src->cur;
        if (src->cur[0] == '\n') {
            ++lineno;
        }
        if (c == ',' || c == '\n' || c == ' ' || c == '\0') {
            cur_token->end = src->cur++;
            if (c == ',')
                src->cur++;
            break;
        }
        ++src->cur;
    }
    if (cur_token->end > src->end) {
        *cur_token = (token_t){0};
        return;
    }
    if (str_len(cur_token->id) == 0)
        goto retry;

    printd("line %d, indent %d: |", cur_token->lineno, cur_token->indent);
    str_fprintnl((str *)cur_token, stdout);
    printd("|\n");

    if (src->cur[-1] == '\n') {
        unsigned char new_indent = 0;
        while (src->cur[0] == ' ') {
            src->cur++;
            ++new_indent;
        }
        if (indent % 4 != 0) {
            compile_err(cur_token, "indentation should be in mutliple of 4\n");
        }
        if (new_indent > indent) {
            context->cur_token.eob = SOB;
        }

        if (new_indent < indent) {
            context->cur_token.eob = EOB;
        }
        indent = new_indent;
    }
}

#define CSC_RED "\x1b[31m"
#define CSC_RESET "\x1b[0m"

#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 2, 3)))
#endif
void compile_err(const token_t *token, const char *format, ...) {
    has_compile_err = true;
    fputs(CSC_RED, stderr);
    fprintf(stderr, "error in line %d: ", token->lineno);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs(CSC_RESET, stderr);
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 1, 2)))
#endif
void compile_warning(const char *format, ...) {
    fputs(CSC_RED, stderr);
    fprintf(stderr, "warning in line %d: ", lineno);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs(CSC_RESET, stderr);
}

void str_printerr(str s) {
    fputs(CSC_RED, stderr);
    str_fprint(&s, stderr);
    fputs(CSC_RESET, stderr);
}


void literal_string(const parser_context *restrict context, const token_t *restrict token) {
    bool escape = emit_need_escaping();
    if (token->end[-1] != '"') {
        compile_err(token, "expected closing \"\n");
    }
    if (!escape) {
        emit_string_lit(context->reg.type, context->reg.offset, (str *)token);
        return;
    }
    str token_str = (str){token->data, token->end};
    size_t len = str_len(token_str);
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
                    compile_err(token, "%"PRId64" is too large for a string literal", number);
                } else {
                    result = (char)number;
                }
                break;
            }
            *unescaped.cur++ = result;
        }
    }

    str unescaped_s = str_from_iter(&unescaped);
    emit_string_lit(context->reg.type, context->reg.offset, &unescaped_s);
    free(unescaped.start);
}

opt_i64 lit_numeric(const token_t *token) {
    i64 value = 0;
    if (isdigit(token->data[0])) {
        value = strtoll(token->data, NULL, 0);
    } else if (token->data[0] == '\'') {
        char c = token->data[1];
	if (token->end[-1] != '\'') {
	    compile_err(token, "expected closing \'\n");
	}
	value = c;
    } else if (str_eq_lit(&token->id, "true")) {
        value = 1;
    } else if (str_eq_lit(&token->id, "false")) {
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
static const reg_t FP = (reg_t){ .type = FRAME };

regable read_regable(token_t _token) {
    token_t *token = &_token;
    regable result = (regable){ .value = 0, .tag = NONE};
    if (isupper(token->data[0]) || token->data[0] == '^') {
        int scope_up = 0;
        while (token->data[0] == '^') {
            scope_up += 1;
            ++token->data;
        }
        reg_t *e;
        if (!find_id(&local_ids, token, &e, scope_up)
            || e->type == RD_NONE) {
            compile_err(token, "unknown id "), str_fprint((str *)token, stderr);
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

void binary_op(const regable *restrict lhs, parser_context *restrict context) {
    token_t lhs_token = context->cur_token;
    lex(context);
    token_t op_token = context->cur_token;

    if (streq(op_token.data, "=[")) {
        op_token.data += 2, op_token.end -= 1;
        regable rhs = read_regable(op_token);
        if (rhs.tag != REG) {
            compile_err(&op_token, "register required for store target\n");
            return;
        }
        reg_t reg_to_store;
        if (lhs->tag == VALUE) {
            reg_to_store = (reg_t){.type = SCRATCH, .offset = context->reg.offset};
            emit_mov(reg_to_store, lhs->value);
        } else if (lhs->tag == REG) {
            reg_to_store = lhs->reg;
        } else {
            unreachable;
        }
        emit_str_fp(reg_to_store, rhs.reg.offset);
        return;
    }

    lex(context);
    token_t rhs_token = context->cur_token;
    regable rhs = read_regable(rhs_token);

    if (rhs.tag == NONE) {
        compile_err(&rhs_token, "expected operand, but found "), str_print((str *)&rhs_token);
    }
    if (op_token.data[0] == '+') {
        if (lhs->tag == VALUE && rhs.tag == VALUE) {
            emit_mov(context->reg, lhs->value + rhs.value);
        } else if (lhs->tag == VALUE && rhs.tag == REG) {
            emit_add(context->reg, rhs.reg, lhs->value);
        } else if(lhs->tag == REG && rhs.tag == VALUE) {
            emit_add(context->reg, lhs->reg, rhs.value);
        } else if (lhs->tag == REG && rhs.tag == REG) {
            emit_add_reg(context->reg, lhs->reg, rhs.reg);
        } else unreachable;
    } else if (op_token.data[0] == '-') {
        if (lhs->tag == VALUE && rhs.tag == VALUE) {
            emit_mov(context->reg, lhs->value - rhs.value);
        } else if (lhs->tag == VALUE && rhs.tag == REG) {
            int tmp_reg_off = context->reg.offset;
            if (context->reg.type == SCRATCH) {
                tmp_reg_off += 1;
            }
            emit_mov((reg_t){SCRATCH, tmp_reg_off}, lhs->value);
            emit_sub_reg(context->reg, (reg_t){SCRATCH, tmp_reg_off}, rhs.reg);
        } else if(lhs->tag == REG && rhs.tag == VALUE) {
            emit_sub(context->reg, lhs->reg, rhs.value);
        } else if (lhs->tag == REG && rhs.tag == REG) {
            emit_sub_reg(context->reg, lhs->reg, rhs.reg);
        } else unreachable;
    } else if (streq(op_token.data, "is")) {
        if (lhs->tag == VALUE) {
            compile_err(&lhs_token, "a register is expected for the left hand side of the operator\n");
        }
        if (rhs.tag == VALUE)
            emit_cmp(lhs->reg, rhs.value);
        else if (rhs.tag == REG)
            emit_cmp_reg(lhs->reg, rhs.reg);
        else unreachable;
        if (is_id(rhs_token.end[1])) {
            // ternary operator.
            lex(context);
            token_t jump_target = context->cur_token;
            if (!streq(jump_target.end - 2, "->")) {
                compile_err(&jump_target, "-> expected at the end of a conditional branch");
            }
            jump_target.end -= 2;
            emit_branch_cond(COND_EQ, context->name, &jump_target);
        }
    } else {
        compile_err(&op_token, "unknown operator "), str_print((str *)&op_token);
    }
}

bool expr(parser_context *context) {
    token_t _token = context->cur_token;
    token_t *token = &_token;
    if (token->data[0] == '"') {
        literal_string(context, token);
        return true;
    }
    if (token->data[0] == '[') {
        token->data += 1, token->end -= 1;
        int scope_up = 0;
        while (token->data[0] == '^') {
            scope_up += 1;
            ++token->data;
        }
        reg_t *e;
        if (!find_id(&local_ids, token, &e, scope_up))
            return false;
        if (token->end[0] != ']') {
            compile_err(token, "closing ']' expected(expr)\n");
        }
        if (e->type != NONE)
            emit_ldr_fp(context->reg, e->offset);
        return true;
    }

    regable lhs = read_regable(*token);
    if (lhs.tag == NONE) {
        return false;
    }

    if (token->end[0] != ',' && token->end[0] != '\n' && !streq(token->end + 1, "=[]")) {
        binary_op(&lhs, context);
    } else {
        if (lhs.tag == VALUE) {
            emit_mov(context->reg, lhs.value);
        } else if (lhs.tag == REG) {
            const reg_t *nreg = &lhs.reg;
            if (nreg->type == NREG) {
                emit_mov_reg(context->reg, lhs.reg);
            } else if (nreg->type == STACK) {
                emit_sub(context->reg, FP, nreg->offset);
            } else {
                unreachable;
            }
        }
    }
    return true;
}

bool expr_line(parser_context *context) {
    token_t *token = &context->cur_token;
    bool ok = expr(context);
    if (!ok)
        return false;
    context->reg.offset++;

    while (token->end[0] == ',' && isspace(token->end[1])) {
        printd(", ");
        lex(context);
        *token = context->cur_token;
        str_printd((str *)token);
        ok = expr(context);
        if (!ok)
            break;
        context->reg.offset++;
    }
    if (context->cur_token.end[0] == '\n') {
        context->reg.offset = 0;
        context->reg.type = SCRATCH;
    }
    return true;
}

bool stmt(parser_context *restrict context) {
    token_t _token = context->cur_token;
    token_t *token = &_token;
    if (!streq(token->end, " ::")) {
        return false;
    }

    if (isupper(token->data[0])) {
        lex(context);
        if (context->cur_token.end[0] != '\n') {
            context->reg.type = NREG;
        }
        printf("decl nreg\n");
        reg_t *reg = overwrite_id(*local_ids.cur, token, &(reg_t){NREG, context->nreg_count});
        context->reg.offset = context->nreg_count++;
        arr_target_push(&context->targets, (target){.reg = reg});
        return true;
    } else if (token->data[0] == '[') {
        token->data += 1;
        token->end -= 1;
        if (token->end[0] != ']') {
            compile_err(token, "closing ']' expected(stmt)\n");
        }
        context->reg.type = SCRATCH;
        context->stack_size += sizeof (i32);
        int offset = context->stack_size;
        printf("decl stack\n");
        reg_t *reg = overwrite_id(*local_ids.cur, token, &(reg_t){STACK, offset});
        arr_target_push(&context->targets, (target){.reg = reg});

        lex(context);
        return true;
    }
    return false;
}

// out_param_names: set to NULL if not needed
symbol_t *label_meta(parser_context *context, arr_str *out_param_names) {
	token_t _token = context->cur_token;
    token_t *token = &_token;
    str label = { .data = token->data, .end = token->end - 1 };

    symbol_t symbol = (symbol_t) {
        .name = label,
        .airity = 0,
        .is_fn = false,
    };
    if (out_param_names) {
        arr_str_init(out_param_names);
    }

	if (streq(token->end, " (")) {
        indent += 4;
        arr_mini_hashset_push(&local_ids); // TODO maybe move this to stmt_label

        lex(context);
        _token = context->cur_token;
        token->data += 1;
        bool parsing_arg = true;
		while (token->end < context->src->end) {
            bool break_out = false;
            if (token->end[-1] == ')') {
                break_out = true;
                token->end -= 1;
            }
			if (isupper(token->data[0])) {
                if (parsing_arg) {
                    symbol.airity += 1;
                    if (out_param_names)
                        arr_str_push(out_param_names, token->id);
                }
			} else if (streq(token->data, "=>")) {
                parsing_arg = false;
                symbol.is_fn = true;
			}
            if (break_out)
                break;
            lex(context);
            token = &context->cur_token;
		}
	}

    hashmap_entry *entry = hashmap_find(fn_ids, label);
    symbol_t *symbol_existing = &entry->value;

    if (hashmap_entry_valid(entry)) {
        if (symbol_existing->airity != symbol.airity || symbol_existing->is_fn != symbol.is_fn) {
            compile_err(token, "incorrect redefinition of fn "), str_printerr(label);
            return NULL;
        }
    } else {
        entry->key = label;
        entry->value = symbol;
    }

    return symbol_existing;
}

void stmt_label(parser_context *context) {
    arr_str params;
    symbol_t *symbol = label_meta(context, &params);
    if (symbol == NULL) {
        return;
    }

    if (!symbol->is_fn) {
        emit_label(context->name, symbol->name);
    }

    for (int i = 0; i < symbol->airity; ++i) {
        reg_t arg_reg = {.type = PARAM, .offset = i};
        reg_t r = {NREG, context->nreg_count++};
        emit_mov_reg(r, arg_reg);
        str param_name = params.data[i];
        if (!add_id(*local_ids.cur, param_name, &r)) {
            compile_err(&context->cur_token, "parameter ids should be unique\n");
        }
    }

    if (symbol->is_fn) {
        context->name = symbol->name;
        emit_fn(symbol->name);
    }
}

bool directives(parser_context *context) {
    const token_t *token = &context->cur_token;
    if (token->data[0] != '#')
        return false;

    str token_str = { .data = token->id.data + 1, .end = token->id.end };
    if (str_eq_lit(&token_str, "declare")) {
        printf("declare directive\n");
        lex(context);
        symbol_t *symbol = label_meta(context, NULL);
        if (symbol == NULL) {
            return true;
        }
    }
    return true;
}

void parse(parser_context *context) {
    const token_t *token = &context->cur_token;
    str *token_str = &(str){.data = token->data, .end = token->end};
    if (directives(context)) {

    } else if (stmt(context)) {

    } else if (expr_line(context)) {

    } else if (str_eq_lit(token_str, "=[]")) {
        target *cur_target = arr_target_top(&context->targets);
        if (cur_target == NULL || cur_target->reg->type != STACK) {
            compile_err(token, "nothing to store to\n");
            return;
        }
        reg_t src = (reg_t){.type = context->reg.type, .offset = context->reg.offset};
        emit_str_fp(src, cur_target->reg->offset);
        cur_target->target_assigned = true;
    } else if (str_eq_lit(token_str, "=")) {
        target *cur_target = arr_target_top(&context->targets);
        if (cur_target == NULL || cur_target->reg->type != NREG) {
            compile_err(token, "nothing to assign\n");
            return;
        }
        context->reg = *cur_target->reg;
        cur_target->target_assigned = true;
    } else if (str_eq_lit(token_str, "ret")) {
        context->reg.type = RET;
        lex(context);
        expr_line(context);
        if (context->indent == context->cur_token.indent) {
            context->ended = true;
        } else {
            context->has_branched_ret = true;
            emit_branch(context->name, STR_FROM("ret"));
        }
    } else if (str_ends_with(token_str, "=>")) {
        str fn_name = (str){token->data, token->end - 2};
        if (!str_empty(&context->deferred_fn_call) && str_empty(&fn_name)) {
            fn_name = str_move(&context->deferred_fn_call);
        } else {
            compile_err(token, "empty function name");
        }
        hashmap_entry *entry = hashmap_tryfind(fn_ids, fn_name);
        if (entry == NULL) {
            compile_err(token, "trying to call undefined function "), str_printerr(fn_name);
        } else {
            symbol_t *s = &entry->value;
            int arg_counts = context->reg.offset;
            if (arg_counts != s->airity) {
                compile_err(token, "expected argument count %d, but found %d\n", s->airity, arg_counts);
            }
            emit_fn_call(&fn_name);
        }
        context->reg.offset = 0;
        context->reg.type = SCRATCH;
        context->calls_fn = true;
    } else if (islower(token->data[0]) || token->data[0] == '_') {
        if (streq(token->end - 2, "->")) {
            emit_branch(context->name, (str){.data = token->data, .end = token->end - 2});
        } else if (streq(token->end - 1, ":")) {
			stmt_label(context);

            context->indent = indent;
        } else if (isalnum(token->end[-1]) || token->end[-1] == '_') {
            context->reg.type = PARAM;
            context->deferred_fn_call = token->id;
        } else {
            compile_err(token, "unknown token "), str_fprint((str *)token, stderr);
        }
    } else {
        compile_err(token, "unknown token "), str_fprint((str *)token, stderr);
    }
}

void function(iter *src, FILE *object_file) {
    emit_reset_fn();
    arr_mini_hashset_init(&local_ids);

    parser_context *context = &(parser_context){
        .src = src,
        .reg = (reg_t) {.type = SCRATCH, .offset = 0 },
        .name = str_null,
    };
    if (src->cur == src->start) {
        context->name = STR_FROM("main");
        emit_fn(context->name);
    }

    context->indent = context->cur_token.indent;
    arr_target_init(&context->targets);

    while (src->cur < src->end) {
        lex(context);
        token_t *cur_token = &context->cur_token;
        if (str_len(cur_token->id) == 0) {
            continue;
        }
        parse(context);

        if (cur_token->eob == SOB) {
            printd("\nstart of a block\n");
            arr_mini_hashset_push(&local_ids);
        } else if (cur_token->eob == EOB) {
            target *cur_target = arr_target_top(&context->targets);
            if (cur_target && !cur_target->target_assigned) {
                if (cur_target->reg->type == STACK)
                    compile_err(cur_token, "this block must store\n");
                else
                    compile_err(cur_token, "this block must assign\n");
            }

            arr_target_pop(&context->targets);
            arr_mini_hashset_pop(&local_ids);
            printd("end of a block\n\n");
        }

        if (context->ended) {
            break;
        }
    }

    if (str_len(context->name) == 0) {
        return;
    }

    if (context->has_branched_ret) {
        emit_label(context->name, STR_FROM("ret"));
    }
    emit_fn_prologue_epilogue(context);
    emit_ret();
    emit_fnbuf(object_file);
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

    iter src = { .start = source_start, .cur = source_start, .end = source_start + source_len };

    emit_init();
    while (src.cur < src.end) {
        function(&src, object_file);
    }

    emit_cstr(object_file);

    if (has_compile_err)
        fprintf(stderr, CSC_RED"compilation failed"CSC_RESET);
    return has_compile_err;
}

