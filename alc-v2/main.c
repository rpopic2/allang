#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dyn.h"
#include "emit.h"
#include "err.h"
#include "hashmap.h"
#include "mini_hashset.h"
#include "opt.h"
#include "str.h"
#include "types.h"

#define INT_SIZE 4

void parse_block(parser_context *context);

OPT_GENERIC(i64)

unsigned char lineno = 1;
unsigned char indent = 0;
bool eof = false;
bool has_compile_err = false;
bool do_airity_check = true;

// #define array_len ('Z' - 'A' + 1)


DYN_GENERIC(type_t)

enum type_kind {
    TK_NONE, TK_FUND, TK_ARRAY, TK_STRUCT, TK_UNION,
};
typedef u8 type_kind;

typedef struct _type_t {
    size_t size;
    u8 align;
    u8 addr;
    type_kind tag;
    sign_t sign;
    union {
        struct {
            usize len;
            type_t *type;
        } arr;
        struct {
            dyn_T members;
        } struct_t;
    };
} type_t;

type_t *type_i32;
type_t *type_comptime_int = &(type_t){.align = 0, .sign = S_SIGNED, .size = 0, .tag = TK_NONE};

HASHMAP_GENERIC(symbol_t, 100, hashmap_hash)
HASHMAP_GENERIC(type_t, 70, type_hash)

hashmap_symbol_t fn_ids;
hashmap_type_t types;


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
            ++src->cur;
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
        eof = true;
        return;
    }
    if (str_len(cur_token->id) == 0)
        goto retry;

    printd("line %d, indent %d: |", cur_token->lineno, cur_token->indent);
    str_printdnl((str *)cur_token);
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

#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 2, 3)))
#endif
void compile_err(const token_t *token, const char *format, ...) {
    has_compile_err = true;
    fputs(CSI_RED, stderr);
    if (token) {
        fprintf(stderr, "error in line %d: ", token->lineno);
    }

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs(CSI_RESET, stderr);
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 1, 2)))
#endif
void compile_warning(const char *format, ...) {
    fputs(CSI_RED, stderr);
    fprintf(stderr, "warning in line %d: ", lineno);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs(CSI_RESET, stderr);
}

void str_printerr(str s) {
    fputs(CSI_RED, stderr);
    str_fprint(&s, stderr);
    fputs(CSI_RESET, stderr);
}


void literal_string(parser_context *restrict context, const token_t *restrict token) {
    bool escape = emit_need_escaping();
    if (token->end[-1] != '"') {
        compile_err(token, "expected closing \"\n");
    }
    if (!escape) {
        context->reg.size = sizeof (char *);
        context->reg.addr = 1;
        context->reg.type = hashmap_type_t_tryfind(types, STR_FROM("u8"));
        emit_string_lit(context->reg, (str *)token);
        return;
    }
    str token_str = (str){token->data, token->end};
    size_t len = str_len(token_str);
    char *raw = malloc(len);
    if (!raw)
        malloc_failed();
    iter unescaped = iter_init(raw, len);

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
    emit_string_lit(context->reg, &unescaped_s);
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
static const reg_t FP = (reg_t){ .reg_type = FRAME, .size = sizeof (void *) };

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
            || e->reg_type == RD_NONE) {
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
    if (top_reg.reg_type == lhs.reg.reg_type
            && top_reg.offset == lhs.reg.offset
            && !top->target_assigned) {
        const token_t *token = &context->cur_token;
        compile_err(token, "use of unassigned register "), str_printerr(token->id);
    }
}

void read_load_store_offset(parser_context *context, str s, reg_t *out_reg, regable *out_offset) {
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

        } else if (offset_regable.tag == REG && offset_regable.reg.reg_type == NREG) {

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
    if (reg.reg_type != STACK && reg.addr <= 0) {
        compile_err(cur_token, "a register conatining addr is expected\n");
    }
    if (offset_regable.tag == VALUE) {
        size_t stride;
        if (reg.type == NULL) {
            compile_err(cur_token, "compiler bug: reg type was NULL\n");
            printf("reg type: %d", reg.reg_type);
            stride = sizeof (i32);
        } else {
            stride = reg.type->size;
        }
        offset_regable.value *= stride;

        if (reg.reg_type == STACK) {
            offset_regable.value += reg.offset;
            offset_regable.value = -offset_regable.value;
            reg = (reg_t){
                .reg_type = FP.reg_type, .size = FP.size,
                .type = reg.type,
            };
        }
    } else if (offset_regable.tag == REG) {
        if (reg.reg_type == STACK) {
            compile_err(&context->cur_token, "offset of stack variable by register is not allowed\n");
        }
    } else unreachable;
    *out_offset = offset_regable;
    *out_reg = reg;
    return;
}

void reg_typecheck(const token_t *token, reg_t lhs, reg_t rhs) {
    if (lhs.type == rhs.type)
        return;
    compile_err(token, "type checker:\n");

    size_t lsize = lhs.size;
    size_t rsize = rhs.size;
    if (lhs.addr != rhs.addr) {
        if (rhs.addr) {
            compile_err(token, "\t- address of %d indirection(s) expected, but found %d indirection(s)\n", lhs.addr, rhs.addr);
        }
    }
    if (lsize != rsize) {
        compile_err(token, "\t- register of size %zd expected, but was %zd\n", lsize, rsize);
    }
    bool lsign = lhs.type->sign;
    bool rsign = rhs.type->sign;
    if (lsign != rsign) {
        if (lsign) {
            compile_err(token, "\t- expected signed, but found unsigned\n");
        } else {
            compile_err(token, "\t- expected unsigned, but found signed\n");
        }
    }
}

void binary_op(const regable *restrict lhs, parser_context *restrict context) {
    token_t lhs_token = context->cur_token;
    lex(context);
    token_t op_token = context->cur_token;

    if (streq(op_token.data, "=[")) {
        str s = op_token.id;
        s.data += 1;
        reg_t rhs;
        regable offset;
        read_load_store_offset(context, s, &rhs, &offset);

        reg_t reg_to_store;
        if (lhs->tag == VALUE) {
            reg_to_store = (reg_t){
                .reg_type = SCRATCH, .offset = context->reg.offset,
                .size = rhs.size
            };
            emit_mov(reg_to_store, lhs->value);
            if (rhs.type == NULL) {
                rhs.type = type_i32;
            }
        } else if (lhs->tag == REG) {
            reg_to_store = lhs->reg;
            rhs.type = lhs->reg.type;
        } else {
            unreachable;
        }

        printd("binary_op:store\n");
        if (offset.tag == REG) {
            emit_str_reg(reg_to_store, rhs, offset.reg);
        } else {
            emit_str(reg_to_store, rhs, (int)offset.value);
        }
        return;
    }

    lex(context);
    token_t rhs_token = context->cur_token;
    regable rhs = read_regable(rhs_token.id, &rhs_token);

    context->reg.size = INT_SIZE;
    context->reg.type = NULL;
    if (lhs->tag == VALUE) {
        if (rhs.tag == REG) {
            context->reg.size = rhs.reg.size;
            context->reg.type = rhs.reg.type;
        } else if (rhs.tag == VALUE) {
            context->reg.type = type_comptime_int;
        }
    } else if (lhs->tag == REG) {
        if (rhs.tag == REG) {
            reg_typecheck(&rhs_token, lhs->reg, rhs.reg);
        }
        context->reg.size = lhs->reg.size;
        context->reg.type = rhs.reg.type;
    }

    if (rhs.tag == NONE) {
        compile_err(&rhs_token, "expected operand, but found "), str_printerr(rhs_token.id);
        return;
    } else if (rhs.tag == REG && rhs.reg.reg_type == NREG) {
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
            if (context->reg.reg_type == SCRATCH) {
                tmp_reg_off += 1;
            }
            emit_mov((reg_t){.reg_type = SCRATCH, .offset = tmp_reg_off}, lhs->value);
            emit_sub_reg(context->reg, (reg_t){.reg_type = SCRATCH, .offset = tmp_reg_off}, rhs.reg);
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
            str name = STR_FROM("lbb");
            int index = context->unnamed_labels++;
            emit_branch_cond(COND_EQ, context->name, name, index);
            lex(context);
            parse_block(context);
            emit_label(context->name, name, index);
        }
    } else {
        compile_err(&op_token, "unknown operator "), str_printerr(op_token.id);
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
        reg_t *lhs = &context->reg;
        reg_t rhs;
        regable offset;
        read_load_store_offset(context, token->id, &rhs, &offset);
        if (!rhs.type) {
            compile_err(token, "compiler bug: type you are trying to load is null\n");
            return true;
        }
        if (rhs.type->size > MAX_REG_SIZE) {
            compile_err(&context->cur_token, "cannot load object of size bigger than 16 bytes to register\n");
        }
        lhs->size = (reg_size)rhs.type->size;
        lhs->type = rhs.type;
        if (offset.tag == VALUE) {
            emit_ldr(*lhs, rhs, (int)offset.value);
        } else {
            emit_ldr_reg(*lhs, rhs, offset.reg);
        }
        context->reg.type = rhs.type;
        return true;
    }

    regable lhs = read_regable(token->id, token);
    if (lhs.tag == NONE) {
        return false;
    }
    if (lhs.tag == REG && lhs.reg.reg_type == NREG) {
        check_unassigned(lhs, context);
    }

    if (token->end[0] != ',' && token->end[0] != '\n' && !streq(token->end + 1, "=[]") && !streq(token->end + 1, "=>")) {
        binary_op(&lhs, context);
    } else {
        if (lhs.tag == VALUE) {
            context->reg.size = 0;
            context->reg.type = type_comptime_int;
            context->reg.addr = 0;
            emit_mov(context->reg, lhs.value);
        } else if (lhs.tag == REG) {
            const reg_t *nreg = &lhs.reg;
            if (nreg->reg_type == NREG) {
                context->reg.size = nreg->size;
                context->reg.type = nreg->type;
                assert(nreg->type);
                emit_mov_reg(context->reg, lhs.reg);
            } else if (nreg->reg_type == STACK) {
                context->reg.size = sizeof (void *);
                context->reg.addr = 1;
                if (nreg->type == NULL) {
                    compile_err(token, "taking address of stack object with unknown type\n");
                }
                context->reg.type = nreg->type;
                emit_sub(context->reg, FP, nreg->offset);
            } else {
                unreachable;
            }
        } else {
            unreachable;
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
        str_printd(&token->id);
        ok = expr(context);
        if (!ok)
            break;
        context->reg.offset++;
    }
    int expr_count = context->reg.offset;
    if (context->cur_token.end[0] == '\n') {
        context->reg.offset = 0;
        context->reg.reg_type = SCRATCH;
    }
    return expr_count;
}

bool expect(parser_context *context, str expected) {
    if (!str_eq(context->cur_token.id, expected)) {
        compile_err(&context->cur_token, "expected "), str_printerr(expected);
        return false;
    }
    return true;
}

void stmt_struct(parser_context *context) {
    type_t _s = {0};
    printf("struct "),str_print(&context->symbol->name);
    str *type_name = &context->symbol->name;
    type_t *s = hashmap_type_t_tryadd(types, *type_name, &_s);
    if (!s) {
        s = &_s;
        compile_err(&context->cur_token, "struct with same name already exist: "), str_printerr(*type_name);
    }
    lex(context);
    expect(context, STR_FROM("{"));
    str *current = &context->cur_token.id;
    lex(context);
    while (!str_eq(*current, STR_FROM("}"))) {
        s->size += sizeof (i32);
        lex(context);
    }
    printf("sturct size was: %zd\n", s->size);
}

target *get_current_target(parser_context *context) {
    const token_t *token = &context->cur_token;
    target *cur_target = arr_target_top(&context->targets);
    if (cur_target == NULL || cur_target->reg->reg_type != NREG) {
        compile_err(token, "nothing to assign\n");
        return NULL;
    }
    return cur_target;
}


bool stmt_stack_store(parser_context *context) {
    const token_t *token = &context->cur_token;
    const str *token_str = &token->id;
    if (!str_eq_lit(token_str, "=[]")) {
        return false;
    }

    target *cur_target = arr_target_top(&context->targets);
    reg_t *target_reg = cur_target->reg;
    if (cur_target == NULL || target_reg->reg_type != STACK) {
        compile_err(token, "nothing to store to\n");
    }
    if (!cur_target)
        return true;

    reg_t src = context->reg;
    src.offset -= 1;

    context->stack_size += context->reg.size;
    int offset = context->stack_size;
    offset = ALIGN_TO(offset, context->reg.size); // TODO not the real way to get align

    target_reg->offset = offset;
    target_reg->size = src.size;
    if (target_reg->type == NULL) {
        if (src.type == type_comptime_int) {
            src.type = type_i32;
            src.size = (reg_size)type_i32->size;
        }
        target_reg->type = src.type;
    }
    emit_str(src, (reg_t){.reg_type = FRAME }, -target_reg->offset);
    cur_target->target_assigned = true;

    if (context->cur_token.end[0] == '\n') {
        context->reg.offset = 0;
        context->reg.reg_type = SCRATCH;
    }

    return true;
}

bool decl_vars(parser_context *context) {
    const token_t *token = &context->cur_token;
    if (!streq(token->end, " ::")) {
        return false;
    }

    if (isupper(token->data[0])) {
        str name = token->id;
        lex(context);
        bool one_liner = context->cur_token.end[0] != '\n';
        if (one_liner) {
            context->reg.reg_type = NREG;
            context->reg.offset = context->nreg_count;
            lex(context);
            expr_line(context);
        }
        reg_t arg = {
            .reg_type = NREG, .offset = context->nreg_count,
            .size = context->reg.size,
            .addr = context->reg.addr, .type = context->reg.type,
        };
        reg_t *reg = overwrite_id(*local_ids.cur, name, &arg);
        context->nreg_count += 1;
        if (!one_liner) {
            target *t = arr_target_push(&context->targets, (target){.reg = reg});
            printf("parse block start\n");
            parse_block(context);
            if (!t->target_assigned) {
                compile_err(&context->cur_token, "this block must assign\n");
            }
            arr_target_pop(&context->targets);
        }
        return true;
    } else if (token->data[0] == '[') {
        str name = token->id;
        name.data += 1;
        name.end -= 1;
        if (name.end[0] != ']') {
            compile_err(token, "closing ']' expected(stmt)\n");
        }
        if (!isupper(name.data[0])) {
            compile_err(token, "name of stack objects must start with uppercase\n");
        }
        lex(context);
        bool one_liner = context->cur_token.end[0] != '\n';
        context->reg.reg_type = SCRATCH;
        reg_t *reg = overwrite_id(*local_ids.cur, name, &(reg_t){.reg_type = STACK});
        if (one_liner) {
            lex(context);
            expr_line(context);
            arr_target_push(&context->targets, (target){.reg = reg});
            lex(context);
            if (!stmt_stack_store(context)) {
                compile_err(&context->cur_token, "store statement '=[]' expected\n");
            }
            arr_target_pop(&context->targets);
        } else {
            target *t = arr_target_push(&context->targets, (target){.reg = reg});
            parse_block(context);
            if (!t->target_assigned) {
                compile_err(&context->cur_token, "this block must store\n");
            }
            arr_target_pop(&context->targets);
        }
        return true;
    }
    return false;
}

void read_and_check_types(parser_context *context, arr_reg_t *rets) {
    const token_t *token = &context->cur_token;
        reg_t *rets_it = rets->data;

        do {
            lex(context);
            if (!expr(context))
                break;
            if (rets_it < rets->cur) {
                if (context->reg.type == type_comptime_int
                        && rets_it->type->tag == TK_FUND) {
                    context->reg.type = rets_it->type;
                }
                reg_typecheck(&context->cur_token, *rets_it, context->reg);
            }
            rets_it += 1;

            context->reg.offset++;
        } while (token->end[0] == ',' && isspace(token->end[1]));
}

bool stmt(parser_context *context) {
    const token_t *token = &context->cur_token;

    if (str_eq_lit(&token->id, "struct")) {
        stmt_struct(context);
        return true;
    }
    if (str_eq_lit(&token->id, "ret")) {
        context->reg.reg_type = RET;

        int arg_count = 0;
        if (context->cur_token.end[0] != '\n') {
            read_and_check_types(context, &context->symbol->rets);
            arg_count = context->reg.offset;
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
        u16 index = context->unnamed_labels++;
        arr_u16 *stack = &context->deferred_unnamed_br;
        u16 *target;
        if (arr_u16_is_empty(stack)) {
            target = stack->data;
        } else {
            target = stack->cur - 1;
        }
        *target = index;
        emit_branch(context->name, STR_FROM("unnamed"), index);
        return true;
    } else if (streq(token->data, "<<")) {
        int index = *context->deferred_unnamed_br.cur;
        if (index == DEFERRED_NONE) {
            compile_err(token, "unmatched branch merger. expected >> before <<\n");
            return true;
        }
        emit_label(context->name, STR_FROM("unnamed"), index);
        return true;
    }

    return decl_vars(context);
}

// out_param_names: set to NULL if not needed
symbol_t *label_meta(parser_context *context, arr_str *out_param_names) {
	token_t _token = context->cur_token;
    token_t *token = &_token;
    token_t *cur_token = &context->cur_token;
    str label = { .data = token->data, .end = token->end - 1 };

    symbol_t symbol = (symbol_t) {
        .name = label,
    };
    arr_reg_t_init(&symbol.params);
    arr_reg_t_init(&symbol.rets);
    if (out_param_names) {
        arr_str_init(out_param_names);
    }

	if (streq(token->end, " (")) {
        indent += 4;
        arr_mini_hashset_push(&local_ids);

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
            } else if (islower(cur_token->data[0])) {
                u8 addr = 0;
                while (str_eq_lit(&cur_token->id, "addr")) {
                    addr += 1;
                    lex(context);
                }
                type_t *type = hashmap_type_t_tryfind(types, cur_token->id);
                if (!type) {
                    compile_err(cur_token, "unknown type "), str_printerr(cur_token->id);
                    goto next;
                }
                reg_size regsize = (reg_size)type->size;
                if (type->size > MAX_REG_SIZE) {
                    compile_err(cur_token, "sizeof the type (%zd) exceeds max register size limit\n", type->size);
                    regsize = MAX_REG_SIZE;
                }
                reg_t reg = {
                    .size = regsize,
                    .offset = symbol.airity,
                    .type = type,
                    .addr = addr,
                };
                printf("reg %d, %d\n", regsize, type->sign);
                if (parsing_arg) {
                    arr_reg_t_push(&symbol.params, reg);
                } else {
                    arr_reg_t_push(&symbol.rets, reg);
                }
			} else if (streq(token->data, "=>")) {
                parsing_arg = false;
                symbol.is_fn = true;
			} else {
                compile_err(token, "unknown token "), str_printerr(token->id);
            }
            if (break_out)
                break;
next:
            lex(context);
            token = &context->cur_token;
		}
	}

    if (arr_reg_t_len(&symbol.params) != symbol.airity) {
        str_fprintnl(&symbol.name, stdout);
        printf(": expected %zd, but %d\n", arr_reg_t_len(&symbol.params), symbol.airity);
        unreachable;
    }

    hashentry_symbol_t *entry = hashmap_symbol_t_find(fn_ids, label);
    symbol_t *symbol_existing = &entry->value;

    if (hashentry_symbol_t_valid(entry)) {
        if (symbol_existing->airity != symbol.airity || symbol_existing->is_fn != symbol.is_fn) {
            compile_err(token, "incorrect redefinition of fn "), str_printerr(label);
            return NULL;
        }
    } else {
        entry->key = label;
        entry->value = symbol;
        arr_reg_t_dup(&entry->value.params, &symbol.params);
        arr_reg_t_dup(&entry->value.rets, &symbol.rets);
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
        str outer_name = str_null;
        if (context->symbol) {
            outer_name = context->symbol->name;
        } else {
            context->symbol = symbol;
            outer_name = context->symbol->name;
        }
        if (!str_empty(&symbol->name)) {
            puts("ohh");
            str_print(&outer_name);
            emit_label(outer_name, symbol->name, 0);
        }
    }

    if (arr_reg_t_len(&symbol->params) != symbol->airity) {
        printf("expected %zd, but %d\n", arr_reg_t_len(&symbol->params), symbol->airity);
        unreachable;
    }
    for (int i = 0; i < symbol->airity; ++i) {
        reg_t arg_reg = {.reg_type = PARAM, .offset = i};
        reg_t *param = &symbol->params.data[i];
        reg_t r = {
            .reg_type = NREG, .offset = context->nreg_count++,
            .size = param->size,
            .type = param->type,
        };
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
        lex(context);
        symbol_t *symbol = label_meta(context, NULL);
        if (symbol == NULL) {
            return true;
        }
    }
    return true;
}

bool stmt_reg_assign(parser_context *context) {
    const token_t *token = &context->cur_token;
    const str *token_str = &token->id;

    if (!str_eq_lit(token_str, "=")) {
        return false;
    }
    target *cur_target = get_current_target(context);
    if (!cur_target)
        return false;
    cur_target->target_assigned = true;
    reg_t *target_reg = cur_target->reg;
    target_reg->size = context->reg.size;
    target_reg->addr = context->reg.addr;
    target_reg->type = context->reg.type;
    emit_mov_reg(*target_reg, context->reg);
    printd("stmt:reg_assign\n");
    return true;
}

void fn_call(parser_context *context) {
    const token_t *token = &context->cur_token;
    const str token_str = token->id;
    symbol_t *symbol = hashmap_symbol_t_tryfind(fn_ids, token_str);
    if (!symbol) {
        compile_err(token, "trying to use undefined function "), str_printerr(token->id);
        return;
    }
    context->deferred_fn_call = symbol;

    bool multiline = token_str.end[0] == '\n';
    if (multiline) {
        return;
    }

    arr_reg_t *params = &symbol->params;
    context->reg.reg_type = PARAM;
    context->reg.offset = 0;
    read_and_check_types(context, params);

    printf("found %d args\n", context->reg.offset);

    if (context->reg.offset > 0)
        lex(context);

    const str *cur_token_str = &token->id;

    if (!str_eq_lit(cur_token_str, "=>")) {
        compile_err(token, "function call '=>' is expected\n");
    }
    symbol_t *s = context->deferred_fn_call;
    if (!s) {
        compile_err(token, "nothing to call\n");
        return;
    }

    str fn_name = s->name;

    int arg_counts = context->reg.offset;
    if (do_airity_check && arg_counts != s->airity) {
        compile_err(token, "expected argument count %d, but found %d\n", s->airity, arg_counts);
    }
    emit_fn_call(&fn_name);

    context->reg.offset = 0;
    context->reg.reg_type = RET;
    type_t *return_type = s->rets.data[0].type;
    context->reg.type = return_type;
    context->reg.addr = return_type->addr;
    context->reg.size = (reg_size)return_type->size;
    context->calls_fn = true;
}

void parse(parser_context *context) {
    const token_t *token = &context->cur_token;
    if (directives(context)) {

    } else if (stmt(context)) {

    } else if (expr_line(context)) {

    } else if (stmt_stack_store(context)) {

    } else if (stmt_reg_assign(context)) {

    } else if (islower(token->data[0]) || token->data[0] == '_') {
        if (streq(token->end - 2, "->")) {
            str label = {.data = token->data, .end = token->end - 2};
            emit_branch(context->symbol->name, label, 0);
        } else if (streq(token->end - 1, ":")) {
			stmt_label(context);
        } else if (isalnum(token->end[-1]) || token->end[-1] == '_') {
            fn_call(context);
        } else {
            compile_err(token, "trying to reference undefined label\n");
        }
    } else {
        compile_err(token, "unexpected token "), str_printerr(token->id);
    }
}

void start_of_block(parser_context *context) {
    printd("\nstart of a block\n");
    arr_mini_hashset_push(&local_ids);
    arr_u16_push(&context->deferred_unnamed_br, DEFERRED_NONE);
}

void end_of_block(parser_context *context) {
    // target *cur_target = arr_target_top(&context->targets);
    // if (cur_target && !cur_target->target_assigned) {
    //     if (cur_target->reg->reg_type == STACK)
    //         compile_err(&context->cur_token, "this block must store\n");
    //     else
    //         compile_err(&context->cur_token, "this block must assign\n");
    // }

    arr_mini_hashset_pop(&local_ids);
    arr_u16_pop(&context->deferred_unnamed_br);
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
            if (cur_token->indent == start_indent + 4) {
                printd("end of a block ret\n\n");
                return;
            }
            end_of_block(context);
        }
    }
}

void function(iter *src, FILE *object_file) {
    emit_reset_fn();
    arr_mini_hashset_init(&local_ids);

    parser_context *context = &(parser_context){
        .src = src,
        .reg = (reg_t) {.reg_type = SCRATCH, .offset = 0 },
        .symbol = NULL,
        .unnamed_labels = 1,
    };
    arr_u16_init(&context->deferred_unnamed_br);
    bool is_main = src->cur == src->start;
    if (is_main) {
        symbol_t tmp = {
            .airity = 2,
            .ret_airity = 1,
            .is_fn = true,
            .name = STR_FROM("main"),
        };
        context->symbol = hashmap_symbol_t_overwrite(fn_ids, tmp.name, &tmp);
        context->name = context->symbol->name;
        emit_fn(context->name);
    }
    arr_target_init(&context->targets);

    if (!is_main) {
        lex(context);
        if (!context->cur_token.data)
            return;
        stmt_label(context);
    }
    context->indent = context->cur_token.indent;
    printd(CSI_GREEN"\n--- start of label: ");
    str_printd(&context->name);
    printd(CSI_RESET);

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
            printf("end detected\n");
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
    printd("end of fn\n");
}

const char *fund_type_names[] = {
    "u8", "u16", "u32", "u64", "u128", "usize",
    "i8", "i16", "i32", "i64", "i128", "isize",
};
const u8 fund_type_sizes[] = {
    1, 2, 4, 8, 16, sizeof (void *),
    1, 2, 4, 8, 16, sizeof (void *),
};

void register_fund_types(void) {
    size_t fund_types_count = sizeof fund_type_names / sizeof (char *);
    for (size_t i = 0; i < fund_types_count; ++i) {
        str name = STR_FROM(fund_type_names[i]);
        type_t s = {
            .size = fund_type_sizes[i],
            .sign = name.data[0] == 'u' ? false : true,
            .tag = TK_FUND,
            .align = (u8)fund_type_sizes[i],
        };
        type_t *t = hashmap_type_t_overwrite(types, name, &s);
        if (str_eq_lit(&name, "i32")) {
            type_i32 = t;
        }
        printd("reg type "), str_print(&name);
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
    if (!source_start)
        malloc_failed();
	memset(source_start, 0, source_len);

    unsigned long bytes_read = fread(source_start, sizeof (char), source_len, source_file);
    if (bytes_read > source_len) {
		fprintf(stderr, "error: buffer overflow. expected %ld bytes but read %lu bytes\n", source_len, bytes_read);
        exit(EXIT_FAILURE);
    }

    size_t source_name_len = strlen(source_name);
    char *out_name = malloc(source_name_len + 1);
    if (!out_name)
        malloc_failed();
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
    register_fund_types();
    while (src.cur < src.end) {
        function(&src, object_file);
    }

    emit_cstr(object_file);

    if (has_compile_err)
        fprintf(stderr, CSI_RED"compilation failed\n"CSI_RESET);
    return has_compile_err;
}
