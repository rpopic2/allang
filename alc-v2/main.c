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

void parse_block(parser_context *context);

OPT_GENERIC(i64)

unsigned char lineno = 1;
unsigned char indent = 0;
bool has_compile_err = false;
bool do_airity_check = true;


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
        if (c == ',' || c == '\n' || c == ' ' || c == '\0' || c == ';') {
            cur_token->end = src->cur++;
            if (c == ',' || c == ';')
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

        if (context->indent > indent) {
            context->ended = true;
        }
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

regable read_regable(str s, const token_t *token) {
    regable result = (regable){ .value = 0, .tag = NONE};
    if (isupper(s.data[0]) || s.data[0] == '^') {
        int scope_up = 0;
        while (s.data[0] == '^') {
            scope_up += 1;
            ++s.data;
        }
        reg_t *e;
        if (!find_id(&local_ids, s, token, &e, scope_up)
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

void check_unassigned(regable lhs, const parser_context *context) {
    target *top = arr_target_top(&context->targets);
    if (!top)
        return;
    reg_t top_reg = *top->reg;
    if (top_reg.type == lhs.reg.type
            && top_reg.offset == lhs.reg.offset
            && !top->target_assigned) {
        const token_t *token = &context->cur_token;
        compile_err(token, "use of unassigned register "), str_printerr(token->id);
    }
}

void load_store_offset(bool store, reg_t target, str s, parser_context *context) {
    const token_t *cur_token = &context->cur_token;
    regable offset_regable = {.tag = VALUE, .value = 0};
    s.data += 1;
    if (s.end[-1] == ']') {
        s.end -= 1;
    } else if (s.end[0] == ',') {
        lex(context);
        str offset_str = cur_token->id;
        offset_str.end -= 1;
        offset_regable = read_regable(offset_str, cur_token);
        if (offset_regable.tag == VALUE) {

        } else if (offset_regable.tag == REG && offset_regable.reg.type == NREG) {
            printf("reg offset\n");
        } else {
            compile_err(&context->cur_token, "valid offset expected, but found "), str_printerr(context->cur_token.id);
        }
        if (cur_token->id.end[-1] != ']') {
            compile_err(cur_token, "closing ']' expected\n");
        }
    } else if (s.end[-1] != ']') {
        compile_err(cur_token, "closing ']' expected\n");
    }
    regable regable_target = read_regable(s, cur_token);
    if (s.data[-2] != '=')
        check_unassigned(regable_target, context);
    if (regable_target.tag != REG) {
        compile_err(cur_token, "register expected\n");
        return;
    }
    reg_t reg = regable_target.reg;
    if (offset_regable.tag == VALUE) {
        int offset = (i32)offset_regable.value * (signed)sizeof(i32);
        if (reg.type == STACK) {
            offset += reg.offset;
            reg = (reg_t){.type = FRAME };
        }
        if (store)
            emit_str(target, reg, offset);
        else
            emit_ldr(target, reg, offset);
    } else if (offset_regable.tag == REG) {
        if (reg.type == STACK) {
            compile_err(&context->cur_token, "offset of stack variable by register is not allowed\n");
            return;
        }
        if (store)
            emit_str_reg(target, reg, offset_regable.reg);
        else
            emit_ldr_reg(target, reg, offset_regable.reg);
    }
    return;
}

void binary_op(const regable *restrict lhs, parser_context *restrict context) {
    token_t lhs_token = context->cur_token;
    lex(context);
    token_t op_token = context->cur_token;

    if (streq(op_token.data, "=[")) {
        str s = op_token.id;
        s.data += 1;
        reg_t reg_to_store;
        if (lhs->tag == VALUE) {
            reg_to_store = (reg_t){.type = SCRATCH, .offset = context->reg.offset};
            emit_mov(reg_to_store, lhs->value);
        } else if (lhs->tag == REG) {
            reg_to_store = lhs->reg;
        } else {
            unreachable;
        }
        load_store_offset(true, reg_to_store, s, context);
        return;
    }

    lex(context);
    token_t rhs_token = context->cur_token;
    regable rhs = read_regable(rhs_token.id, &rhs_token);

    if (rhs.tag == NONE) {
        compile_err(&rhs_token, "expected operand, but found "), str_print((str *)&rhs_token);
    } else if (rhs.tag == REG && rhs.reg.type == NREG) {
        check_unassigned(rhs, context);
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
            emit_branch_cond(COND_EQ, context->name, jump_target.id, 0);
        } else if (streq(rhs_token.end + 1, "->")) {
            printf("unnamed cond b\n");
            str name = STR_FROM("lbb");
            int index = context->unnamed_labels++;
            emit_branch_cond(COND_EQ, context->name, name, index);
            lex(context);
            parse_block(context);
            emit_label(context->name, name, index);
            printf("end unnamed cond b\n");
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
        load_store_offset(false, context->reg, token->id, context);
        return true;
    }

    regable lhs = read_regable(token->id, token);
    if (lhs.tag == NONE) {
        return false;
    }
    if (lhs.tag == REG && lhs.reg.type == NREG) {
        check_unassigned(lhs, context);
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

int expr_line(parser_context *context) {
    token_t *token = &context->cur_token;
    if (token->data == NULL)
        return 0;
    bool ok = expr(context);
    if (!ok)
        return 0;
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
    int expr_count = context->reg.offset;
    if (context->cur_token.end[0] == '\n') {
        context->reg.offset = 0;
        context->reg.type = SCRATCH;
    }
    return expr_count;
}

bool stmt(parser_context *restrict context) {
    token_t _token = context->cur_token;
    token_t *token = &_token;

    if (str_eq_lit(&token->id, "ret")) {
        printf("ret stmt\n");
        context->reg.type = RET;
        int arg_count = 0;
        if (context->cur_token.end[0] != '\n') {
            lex(context);
            arg_count = expr_line(context);
        }
        if (do_airity_check && arg_count != context->symbol->ret_airity) {
            compile_err(token, "expected to return %d values, but found %d\n",
                    context->symbol->ret_airity, arg_count);
        }
        if (context->cur_token.data == NULL
                || context->indent == context->cur_token.indent) {
            context->ended = true;
            context->last_line_ret = true;
            return true;
        }
        context->has_branched_ret = true;
        emit_branch(context->symbol->name, STR_FROM("ret"), 0);
        return true;
    } else if (streq(token->data, ">>")) {
        printf("branch merge start\n");

        int index = context->unnamed_labels++;
        arr_int *stack = &context->deferred_unnamed_br;
        int *target;
        if (arr_int_is_empty(stack)) {
            target = stack->data;
        } else {
            target = stack->cur - 1;
        }
        *target = index;
        emit_branch(context->name, STR_FROM("unnamed"), index);
        return true;
    } else if (streq(token->data, "<<")) {
        printf("branch merge end\n");
        int index = *context->deferred_unnamed_br.cur;
        if (index == DEFERRED_NONE) {
            compile_err(token, "unmatched branch merger. expected >> before <<\n");
            return true;
        }
        emit_label(context->name, STR_FROM("unnamed"), index);
        return true;
    }

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
                } else {
                    symbol.ret_airity += 1;
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
        emit_label(context->symbol->name, symbol->name, 0);
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
        emit_fn(symbol->name);
        context->symbol = symbol;
        context->name = symbol->name;
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

target *stmt_assign(parser_context *context) {
    const token_t *token = &context->cur_token;
    target *cur_target = arr_target_top(&context->targets);
    if (cur_target == NULL || cur_target->reg->type != NREG) {
        compile_err(token, "nothing to assign\n");
        return NULL;
    }
    return cur_target;
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
        emit_str(src, (reg_t){.type = FRAME }, cur_target->reg->offset);
        cur_target->target_assigned = true;

        if (context->cur_token.end[0] == '\n') {
            context->reg.offset = 0;
            context->reg.type = SCRATCH;
        }
    } else if (str_eq_lit(token_str, "=")) {
        target *cur_target = stmt_assign(context);
        if (!cur_target)
            return;
        context->reg = *cur_target->reg;
        cur_target->target_assigned = true;

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
            if (do_airity_check && arg_counts != s->airity) {
                compile_err(token, "expected argument count %d, but found %d\n", s->airity, arg_counts);
            }
            emit_fn_call(&fn_name);

            if (token_str->end[1] == '=') {
                target *t = stmt_assign(context);
                if (do_airity_check && s->ret_airity != 1) {
                    compile_err(&context->cur_token, "function must return single value to be assigned\n");
                }
                if (t) {
                    emit_mov_reg(*t->reg, (reg_t){RET, 0});
                }
                lex(context);
                char end = context->cur_token.end[0];
                if (end != '\n' && end != ';') {
                    compile_err(&context->cur_token, "expected end of line or ';' after function result assignment");
                }
            }
        }
        context->reg.offset = 0;
        context->reg.type = SCRATCH;
        context->calls_fn = true;
    } else if (islower(token->data[0]) || token->data[0] == '_') {
        if (streq(token->end - 2, "->")) {
            emit_branch(context->symbol->name, (str){.data = token->data, .end = token->end - 2}, 0);
        } else if (streq(token->end - 1, ":")) {
			stmt_label(context);

            // context->indent = indent;
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

void start_of_block(parser_context *context) {
    printd("\nstart of a block\n");
    arr_mini_hashset_push(&local_ids);
    arr_int_push(&context->deferred_unnamed_br, DEFERRED_NONE);
}

void end_of_block(parser_context *context) {
    target *cur_target = arr_target_top(&context->targets);
    if (cur_target && !cur_target->target_assigned) {
        if (cur_target->reg->type == STACK)
            compile_err(&context->cur_token, "this block must store\n");
        else
            compile_err(&context->cur_token, "this block must assign\n");
    }

    arr_target_pop(&context->targets);
    arr_mini_hashset_pop(&local_ids);
    arr_int_pop(&context->deferred_unnamed_br);
    printd("end of a block\n\n");
}

void parse_block(parser_context *context) {
    const token_t *cur_token = &context->cur_token;
    int start_indent = cur_token->indent;
    bool check_start = true;
    while (true) {
        if (cur_token->eob == SOB) {
            start_of_block(context);
        }

        lex(context);
        if (check_start) {
            check_start = false;
            if (cur_token->indent != start_indent + 4) {
                compile_err(cur_token, "indented block expected\n");
            }
        }
        if (str_len(cur_token->id) == 0) {
            return;
        }
        parse(context);

        if (cur_token->eob == EOB) {
            printf("start %d, cur %d\n", start_indent, cur_token->indent);
            if (cur_token->indent == start_indent + 4) {
                printd("end of a block\n\n");
                return;
            }
            end_of_block(context);
        }
    }
}

void function(iter *src, FILE *object_file) {
    printf("\nstart of fn\n");
    emit_reset_fn();
    arr_mini_hashset_init(&local_ids);

    parser_context *context = &(parser_context){
        .src = src,
        .reg = (reg_t) {.type = SCRATCH, .offset = 0 },
        .symbol = NULL,
    };
    arr_int_init(&context->deferred_unnamed_br);
    bool is_main = src->cur == src->start;
    if (is_main) {
        symbol_t tmp = {
            .airity = 2,
            .ret_airity = 1,
            .is_fn = true,
            .name = STR_FROM("main"),
        };
        context->symbol = hashmap_overwrite(fn_ids, tmp.name, &tmp);
        context->name = context->symbol->name;
        emit_fn(context->name);
    }
    arr_target_init(&context->targets);

    if (!is_main) {
        lex(context);
        stmt_label(context);
    }
    context->indent = context->cur_token.indent;
    printf("fn indent: %d", context->indent);

    while (src->cur < src->end) {
        lex(context);
        token_t *cur_token = &context->cur_token;
        if (str_len(cur_token->id) == 0) {
            continue;
        }
        parse(context);

        if (cur_token->eob == SOB) {
            start_of_block(context);
        } else if (cur_token->eob == EOB) {
            end_of_block(context);
        }

        if (context->ended) {
            break;
        }
    }

    if (str_len(context->name) == 0) {
        return;
    }

    if (context->has_branched_ret) {
        emit_label(context->name, STR_FROM("ret"), 0);
    }
    if (do_airity_check && !context->last_line_ret) {
        if (context->symbol->airity != 0) {
            compile_err(&context->cur_token, "expected to return values\n");
        }
    }
    emit_fn_prologue_epilogue(context);
    emit_ret();
    emit_fnbuf(object_file);
    printf("end of fn\n");
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

