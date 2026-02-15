#include <assert.h>
#include <time.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emit.h"
#include "err.h"
#include "hashmap.h"
#include "mini_hashset.h"
#include "opt.h"
#include "str.h"
#include "types.h"
#include "typesys.h"

#define INT_SIZE 4

void parse_block(parser_context *context);
symbol_t *fn_call(parser_context *context);
bool stmt_reg_assign(parser_context *context);
target *get_current_target(parser_context *context);

static const reg_t FP = (reg_t){ .reg_type = FRAME, .rsize = sizeof (void *) };

OPT_GENERIC(i64)

unsigned char lineno = 1;
unsigned char indent = 0;
bool eof = false;
bool has_compile_err = false;
bool do_airity_check = true;

type_t *type_i32;
type_t *type_comptime_int = &(type_t){.align = 0, .sign = S_SIGNED, .size = 0, .tag = TK_NONE, .name = STR_FROM("comptime int")};


u64 hashmap_hash(str id) {
    u64 index = (u64)id.data[0];
    u64 end = (u64)id.end[-1];
    u64 len = str_len(id);
    return index ^ end ^ len;
}

u64 type_hash(str id) {
    u64 index = (u64)id.data[0];
    u64 end = (u64)id.end[-1];
    u64 len = str_len(id);
    return (index ^ end ^ len) | 1;
}

u64 hash_fnv_1a(str id) {
    u64 hash = 0xcbf29ce484222325;
    while (id.data != id.end) {
        hash ^= (u64)*id.data++;
        hash *= 0x100000001b3;
    }
    return hash;
}

HASHMAP_GENERIC(symbol_t, 64, hashmap_hash)
HASHMAP_GENERIC(type_t, 128, type_hash)

hashmap_symbol_t fn_ids;
hashmap_type_t types;

#define E_TOO_BIG_FOR_REG "cannot load object of size bigger than 16 bytes to register\n"

inline static bool is_id(char c) {
    return isalnum(c) || c == '_';
}

u32 next_pow2(u32 n) {
    if (n <= 1) return 1;
    return 1 << (32 - __builtin_clz(n - 1));
}

bool lex(parser_context *context) {
retry:;
    iter *src = context->src;
    token_t *cur_token = &context->cur_token;
    *cur_token = (token_t){.data = src->cur, .end = src->cur};
    cur_token->lineno = lineno;
    cur_token->indent = indent;

    while (true) {
        if (src->cur > src->end) {
            *cur_token = (token_t){.data = src->end, .end = src->end, .eob = true, .indent = indent, .lineno = lineno};
            eof = true;
            return false;
        }
        char c = *src->cur;
        if (c == '"') {
            do {
                c = *(++src->cur);
            } while (c != '"' && c != '\n');
            cur_token->end = ++src->cur;
            if (src->cur[0] == '\n')
                ++lineno;
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
        if (c == ',' || c == '\n' || c == ' ' || c == '\0' || c == ';'
                || c == ')' || c == '(' || c == '{') {
            cur_token->end = src->cur++;
            if (c == ',' || c == ';')
                src->cur++;
            break;
        }
        ++src->cur;
    }
    if (cur_token->end > src->end) {
        *cur_token = (token_t){.data = src->end, .end = src->end, .eob = true, .indent = indent, .lineno = lineno};
        eof = true;
        return false;
    }
    if (str_len(cur_token->id) == 0)
        goto retry;

    printd("line %d, indent %d: |", cur_token->lineno, cur_token->indent);
    str_printdnl((str *)cur_token);
    printd("|\n");

    if (src->cur[-1] == '\n') {
        unsigned char new_indent = 0;
        while (src->cur[0] == '\n') {
            ++lineno;
            src->cur++;
        }
        while (src->cur[0] == ' ') {
            src->cur++;
            ++new_indent;
        }
        if (indent % 4 != 0) {
            compile_err(cur_token, "an indentation should be 4 spaces\n");
        }
        if (new_indent > indent) {
            context->cur_token.eob = SOB;
        }

        if (new_indent < indent) {
            context->cur_token.eob = EOB;
        }
        indent = new_indent;

        if (context->indent > cur_token->indent || context->indent > new_indent) {
            context->ended = true;
        }
    }
    return true;
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

void str_printerrnl(str s) {
    fputs(CSI_RED, stderr);
    str_fprintnl(&s, stderr);
    fputs(CSI_RESET, stderr);
}

void puterr(const char *s) {
    fputs(CSI_RED, stderr);
    fputs(s, stderr);
    fputs(CSI_RESET, stderr);
}


void literal_string(parser_context *restrict context, const token_t *restrict token) {
    bool escape = emit_need_escaping();
    if (token->end[-1] != '"') {
        compile_err(token, "expected closing \"\n");
    }
    if (!escape) {
        context->reg.rsize = sizeof (char *);
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
    if (isdigit(token->data[0]) || token->data[0] == '-') {
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

int extrat_scope_up(str *s) {
    int scope_up = 0;
    while (s->data[0] == '^') {
        scope_up += 1;
        ++s->data;
    }
    return scope_up;
}

regable read_regable(str s, const token_t *token) {
    regable result = (regable){ .value = 0, .tag = NONE};
    if (isupper(s.data[0]) || s.data[0] == '^') {
        if (s.end[-1] == '}')
            s.end--;
        int scope_up = extrat_scope_up(&s);
        reg_t *e;
        if (!find_id(&local_ids, s, token, &e, scope_up)
            || e->reg_type == RD_NONE) {
            compile_err(token, "unknown id "), str_printerr(s);
        } else {
            result.reg = *e;
            result.tag = REG;
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

bool read_load_store_offset(parser_context *context, str s, reg_t *out_reg, regable *out_offset) {
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
        if (offset_regable.tag == NONE) {
            return false;
        } else if (offset_regable.tag == VALUE) {

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

    regable regable_target;
    if (str_eq_lit(&s, "This")) {
        target *t = arr_target_top(&context->targets);
        if (!t) {
            compile_err(cur_token, "nothing to assign\n");
            return false;
        }
        regable_target = (regable){.tag = REG, .reg = *t->reg};
    } else {
        regable_target = read_regable(s, cur_token);
    }
    if (regable_target.tag == NONE) {
        return false;
    }
    if (s.data[-2] != '=')
        check_unassigned(regable_target, context);
    if (regable_target.tag != REG) {
        compile_err(cur_token, "register expected\n");
        return false;
    }
    reg_t reg = regable_target.reg;
    if (reg.reg_type != STACK && reg.addr <= 0) {
        compile_err(cur_token, "a register conatining addr is expected\n");
    }
    if (offset_regable.tag == VALUE) {
        size_t stride;
        if (reg.type == NULL) {
            compile_err(cur_token, "compiler bug: reg type was NULL\n");
            stride = sizeof (i32);
        } else {
            stride = reg.type->size;
        }
        offset_regable.value *= stride;

        if (reg.reg_type == STACK) {
            offset_regable.value += reg.offset;
            offset_regable.value = -offset_regable.value;
            reg = (reg_t){
                .reg_type = FP.reg_type, .rsize = FP.rsize,
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
    return true;
}

void reg_typecheck(const token_t *token, reg_t lhs, reg_t rhs) {
    if (lhs.type == rhs.type && lhs.addr == rhs.addr)
        return;
    type_t *ltype = lhs.type;
    type_t *rtype = rhs.type;
    compile_err(token, "type checker: expected type "), str_printerrnl(ltype ? ltype->name : STR_FROM("NULL")), puterr(", but found "), str_printerr(rtype ? rtype->name : STR_FROM("NULL"));
    if (!ltype || !rtype)
        return;

    if (lhs.addr != rhs.addr) {
        compile_err(token, "\t- address of %d indirection(s) expected, but found %d indirection(s)\n", lhs.addr, rhs.addr);
    }
    size_t lsize = lhs.rsize;
    size_t rsize = rhs.rsize;
    if (lsize != rsize) {
        compile_err(token, "\t- register of size %zd expected, but was %zd\n", lsize, rsize);
    }
    bool lsign = ltype->sign;
    bool rsign = rtype->sign;
    if (lsign != rsign) {
        if (lsign) {
            compile_err(token, "\t- expected signed, but found unsigned\n");
        } else {
            compile_err(token, "\t- expected unsigned, but found signed\n");
        }
    }
}

bool binary_op_store(const regable *restrict lhs, parser_context *restrict context) {
    const token_t *op_token = &context->cur_token;
    if (!streq(op_token->end + 1, "=["))
        return false;
    if (op_token->end[3] == ']')    // =[]. TODO need to merge this with stmt_stack_store?
        return false;

    lex(context);
    str s = op_token->id;
    s.data += 1;
    reg_t rhs;
    regable offset;
    if (!read_load_store_offset(context, s, &rhs, &offset))
        return true;

    reg_t reg_to_store;
    if (lhs->tag == VALUE) {
        reg_to_store = (reg_t){
            .reg_type = SCRATCH, .offset = context->reg.offset,
            .rsize = rhs.rsize
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
    return true;
}

void binary_op(const regable *restrict lhs, parser_context *restrict context) {
    token_t lhs_token = context->cur_token;
    lex(context);
    token_t op_token = context->cur_token;

    lex(context);
    token_t rhs_token = context->cur_token;
    regable rhs = read_regable(rhs_token.id, &rhs_token);

    if (lhs->tag == VALUE) {
        if (rhs.tag == REG) {
            context->reg.rsize = rhs.reg.rsize;
            context->reg.type = rhs.reg.type;
        } else if (rhs.tag == VALUE) {
        }
    } else if (lhs->tag == REG) {
        if (rhs.tag == REG) {
            reg_typecheck(&rhs_token, lhs->reg, rhs.reg);
        }
        context->reg.rsize = lhs->reg.rsize;
        context->reg.type = rhs.reg.type;
    }

    if (rhs.tag == NONE) {
        compile_err(&rhs_token, "expected operand, but found "), str_printerr(rhs_token.id);
        compile_err(&lhs_token, "lhs was: "), str_printerr(lhs_token.id);
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
    } else if (streq(op_token.data, "shl")) {
        if (lhs->tag != REG) {
            compile_err(&lhs_token, "expected register on the left hand side\n");
        }
        if (rhs.tag == VALUE) {
            emit_lsl(context->reg, lhs->reg, rhs.value);
        } else if (rhs.tag == REG) {
            compile_err(&lhs_token, "lslv not implemented\n");
        }
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
        compile_err(&op_token, "unknown binray operator "), str_printerr(op_token.id);
    }
    printd("binary_op\n");
}

void struct_expr(parser_context *context, type_t *type) {
    const token_t *token = &context->cur_token;
    printd(CSI_GREEN"struct expr: "CSI_RESET);
    str_printd(&type->name);
    const str *s = &context->cur_token.id;

    dyn_member_t members = type->struct_t.members;
    dyn_regable args = {0};
    ptrdiff_t member_count = members.cur - members.begin;
    dyn_regable_reserve(&args, member_count + 1);
    bool init_zero = false;
    while (true) {
        if (!lex(context))
            break;
        if (s->data[0] != '.') {
            compile_err(token, "'.' and member name expected in struct literal\n");
        }
        if (streq(s->data, ".. 0")){
            lex(context);
            init_zero = true;
            if (s->end[-1] == '}')
                break;
            continue;
        }

        str member_name = *s;
        member_name.data++;

        member_t *it = members.begin;
        int index = 0;
        for (; it != members.cur; ++it, ++index) {
            if (str_eq(member_name, it->name)) {
                break;
            }
        }
        size_t offset = 0;
        if (it == members.cur) {
            compile_err(token, "member not found: "), str_printerr(member_name);
        } else {
            offset = it->offset;
        }
        (void)offset;

        if (s->end[-1] == '}')
            break;
        if (!lex(context))
            break;

        regable r = read_regable(*s, token);
        args.begin[index] = r;
        if (r.tag == REG) {
            if (r.reg.type != type_comptime_int && it->type != r.reg.type) {
                compile_err(token, "expected type "),
                    str_printerr(it->type->name);
                compile_err(token, "but found "),
                    str_printerr(r.reg.type->name);
            }
        }

        if (s->end[-1] == '}')
            break;
    }

    for (ptrdiff_t i = 0; i < member_count; ++i) {
        regable *r = &args.begin[i];
        if (r->tag != VALUE && r->tag != REG) {
            if (!init_zero) {
                compile_err(token, "a field is not initialized: ");
                str_printerr(members.begin[i].name);
            }
            r->tag = VALUE;
            r->value = 0;
        }
        // printf("\targ %ld: ", i), str_printnl(&members.begin[i].name);
        // printf("\t");
        // if (r->tag == VALUE) {
        //     printf("value: %lld", r->value);
        // } else if (r->tag == REG) {
        //     printf("reg off: %d", r->reg.offset);
        // }
        // printf("\n");
    }
    emit_make_struct(context->reg, type, &args);

    dyn_regable_free(&args);
    printd(CSI_GREEN"\nend struct expr\n"CSI_RESET);
}

bool expr(parser_context *context) {
    bool explicit_type = context->cur_token.end[0] == '{';
    if (explicit_type) {
        str id = context->cur_token.id;
        type_t *type = hashmap_type_t_tryfind(types, id);
        if (type == NULL) {
            compile_err(&context->cur_token, "unknown type "), str_printerr(id);
            goto skip;
        }

        context->reg.type = type;
        context->reg.addr = 0;
        if (type->size > MAX_REG_SIZE) {
            compile_err(&context->cur_token, E_TOO_BIG_FOR_REG);
        }
        context->reg.rsize = (reg_size)type->size;
        if (type->tag == TK_STRUCT) {
            struct_expr(context, type);
            return true;
        }
        lex(context);
    }
skip:;

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
        if (!read_load_store_offset(context, token->id, &rhs, &offset))
            return true;
        if (!rhs.type) {
            compile_err(token, "compiler bug: type you are trying to load is null\n");
            return true;
        }
        if (rhs.type->size > MAX_REG_SIZE) {
            compile_err(&context->cur_token, E_TOO_BIG_FOR_REG);
        }
        lhs->rsize = (reg_size)rhs.type->size;
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

    if (!explicit_type) {
        context->reg.rsize = 0;
        context->reg.type = type_comptime_int;
        context->reg.addr = 0;
    }
    char token_end = token->end[0];
    if (binary_op_store(&lhs, context)) {
        return true;
    }
    if (token_end != ',' && token_end != '\n' && token_end != ')' && !streq(token->end + 1, "=[]") && !streq(token->end + 1, "=>") && !streq(token->end + 1, "=")) {
        binary_op(&lhs, context);
    } else {
        printd("nullary op\n");
        if (lhs.tag == VALUE) {
            emit_mov(context->reg, lhs.value);
        } else if (lhs.tag == REG) {
            const reg_t *nreg = &lhs.reg;
            if (nreg->reg_type == NREG) {
                if (context->reg.reg_type == PARAM) {
                    context->reg.type = nreg->type;
                    context->reg.addr = nreg->addr;
                    context->reg.rsize = nreg->rsize;
                }
                assert(nreg->type);
                emit_mov_reg(context->reg, lhs.reg);
            } else if (nreg->reg_type == STACK) {
                context->reg.rsize = sizeof (void *);
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
    printd("struct "),str_printd(&context->symbol->name);
    str *type_name = &context->symbol->name;
    type_t _s = {.name = *type_name, .tag = TK_STRUCT };
    type_t *s = hashmap_type_t_tryadd(types, *type_name, &_s);
    if (!s) {
        s = &_s;
        compile_err(&context->cur_token, "struct with same name already exist: "), str_printerr(*type_name);
    }
    str *current = &context->cur_token.id;
    while (true) {
        lex(context);
        if (str_empty(current) || current->data[0] == '}')
            break;
        str name = *current;
        lex(context);
        if (str_empty(current) || current->data[0] == '}')
            break;
        type_t *t = hashmap_type_t_tryfind(types, *current);
        if (t == NULL) {
            compile_err(&context->cur_token, "unknown type "), str_printerr(*current);
            continue;
        }
        member_t m = {
            .name = name, .type = t,
            .offset = ALIGN_TO(s->size, t->align),
        };
        s->size = m.offset + t->size;
        s->align = t->align > s->align ? t->align : s->align;
        dyn_member_t_push(&s->struct_t.members, &m);
    }
    s->size = ALIGN_TO(s->size, s->align);
    if (true) {
        printd(CSI_GREEN"struct report for "), str_printd(&s->name);
        printd("=================\n"CSI_RESET);
        printd("\tsize: %zd, align %d\n", s->size, s->align);

        dyn_member_t *members = &s->struct_t.members;
        int ko = 0;
        for (const member_t *it = members->begin; it != members->cur; ++it) {
            const member_t *mem = it;
            printd("\tmember %d: ", ko++);
            str_printdnl(&mem->name);
            printd(" ");
            str_printdnl(&mem->type->name);
            printd("\toffset: %zd, size: %zd\n",
                    mem->offset, mem->type->size);
        }
        printd(CSI_GREEN"end report\n\n"CSI_RESET);
    }
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
    regable reg = {.tag = VALUE};
    reg.reg = context->reg;

    if (!streq(token_str->data, "=[")) {
        return false;
    }

    char next = token_str->data[2];
    target *cur_target;
    if (next == ']') {
        cur_target = arr_target_top(&context->targets);
        if (cur_target == NULL) {
            compile_err(token, "nothing to store to\n");
            return true;
        } else if (cur_target->reg->reg_type != STACK) {
            compile_err(token, "target is not a stack variable\n");
            return true;
        }
    } else if (isupper(next)) {
        str name = *token_str;
        name.data += 2;
        name.end -= 1;

        reg_t *t;
        if (!find_id(&local_ids, name, token, &t, 0)) {
            compile_err(token, "could not find identifier "), str_printerr(name);
        }
        cur_target = &(target){.target_assigned = true, .reg = t};

    } else {
        compile_err(token, "store target expected\n");
        return true;
    }
    reg_t *target_reg = cur_target->reg;

    reg_t src = context->reg;
    src.offset -= 1;
    target_reg->rsize = src.rsize;

    if (src.type == type_comptime_int || src.type == NULL) {
        src.type = type_i32;
        src.rsize = (reg_size)type_i32->size;
    }
    if (target_reg->type == NULL) {
        target_reg->type = src.type;
    }

    int offset;
    if (!cur_target->target_assigned) {
        assert(src.type);
        assert(src.type->size);

        size_t size = next_pow2(src.rsize);
        context->stack_size += size;
        offset = context->stack_size;
    } else {
        offset = target_reg->offset;
    }

    target_reg->offset = offset;

    assert(src.rsize);
    emit_str(src, (reg_t){.reg_type = FRAME }, -offset);
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
            reg_t nreg = {.reg_type = NREG, .offset = context->nreg_count};
            context->reg = nreg;
            lex(context);
            if (expr_line(context)) { }
            else {
                symbol_t *fn = fn_call(context);
                if (fn) {
                    if (fn->ret_airity != 1) {
                        compile_err(&context->cur_token, "this function does not return exactly one value\n");
                    }
                    reg_t ret_reg = fn->rets.data[0];

                    nreg.rsize = ret_reg.rsize;
                    nreg.addr = ret_reg.addr;
                    nreg.type = ret_reg.type;
                    emit_mov_reg(nreg, context->reg);
                }
            }
        }
        if (context->reg.type == type_comptime_int) {
            context->reg.type = type_i32;
        }
        reg_t arg = {
            .reg_type = NREG, .offset = context->nreg_count,
            .rsize = context->reg.rsize,
            .addr = context->reg.addr, .type = context->reg.type,
        };
        reg_t *reg = overwrite_id(*local_ids.cur, name, &arg);
        context->nreg_count += 1;
        if (!one_liner) {
            target *t = arr_target_push(&context->targets, (target){.reg = reg, .name = name});
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
            if (expr_line(context)) { }
            else {
                symbol_t *fn = fn_call(context);
                if (fn) {
                    if (fn->ret_airity != 1) {
                        compile_err(&context->cur_token, "this function does not return exactly one value\n");
                    }
                    context->reg.offset += 1;
                }
            }
            arr_target_push(&context->targets, (target){.reg = reg, .name = name});
            lex(context);
            if (!stmt_stack_store(context)) {
                compile_err(&context->cur_token, "store statement '=[]' expected\n");
            }
            arr_target_pop(&context->targets);
        } else {
            target *t = arr_target_push(&context->targets, (target){.reg = reg, .name = name});
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
                    context->reg.rsize = rets_it->rsize;
                }
                reg_typecheck(&context->cur_token, *rets_it, context->reg);
            }
            rets_it += 1;

            context->reg.offset++;
        } while (token->end[0] == ',' && isspace(token->end[1]));
}

bool stmt_ret(parser_context *context) {
    const token_t *token = &context->cur_token;

    if (!str_eq_lit(&token->id, "ret"))
        return false;

    context->reg.reg_type = RET;

    int arg_count = 0;
    if (context->cur_token.end[0] != '\n') {
        read_and_check_types(context, &context->symbol->rets);
        arg_count = context->reg.offset;
    }
    int expected = context->symbol->ret_airity;
    if (do_airity_check && arg_count != expected) {
        if (islower(token->id.data[0])) {
            symbol_t *fn = fn_call(context);
            arg_count = fn->ret_airity;
            if (arg_count != context->symbol->ret_airity) {
                compile_err(token, "redirected function: expected to return %d values, but found %d\n",
                        context->symbol->ret_airity, arg_count);
            }
        } else {
            compile_err(token, "expected to return %d values (found %d)\n", expected, arg_count);
        }
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
}

bool stmt(parser_context *context) {
    const token_t *token = &context->cur_token;

    if (str_eq_lit(&token->id, "struct")) {
        stmt_struct(context);
        return true;
    }
    if (stmt_ret(context)) {
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
        bool parsing_arg = true;
		while (token->end < context->src->end) {
            bool break_out = false;
            if (token->end[0] == ')') {
                break_out = true;
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
                    if (token->end[0] == ')') {
                        break_out = true;
                    }
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
                    .rsize = regsize,
                    .offset = symbol.airity,
                    .type = type,
                    .addr = addr,
                };
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
            .rsize = param->rsize,
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
        arr_mini_hashset_pop(&local_ids);
        if (symbol == NULL) {
            return true;
        }
    }
    return true;
}

bool stmt_reg_assign(parser_context *context) {
    const token_t *token = &context->cur_token;
    const str *token_str = &token->id;

    char next = token_str->data[1];
    if (!streq(token_str->data, "=") || !(isupper(next) || isspace(next) || next == '^')) {
        return false;
    }
    target *cur_target;
    if (isspace(next) || streq(token_str->data + 1, "This")) {
        cur_target = get_current_target(context);
    } else {
        str name = *token_str;
        name.data += 1;
        reg_t *t;
        int scope_up = extrat_scope_up(&name);
        if (!find_id(&local_ids, name, token, &t, scope_up)) {
            compile_err(token, "could not find identifier "), str_printerr(name);
            return true;
        }
        cur_target = &(target){.target_assigned = true, .reg = t};
    }
    if (!cur_target)
        return false;
    reg_t *target_reg = cur_target->reg;
    reg_t src_reg = context->reg;
    if (src_reg.reg_type == SCRATCH)
        src_reg.offset -= 1;
    if (!cur_target->target_assigned) {
        if (src_reg.type == NULL) {
            compile_err(token, "compiler bug: reg type shouldn't be null\n");
        } else if (src_reg.type == type_comptime_int) {
            src_reg.type = type_i32;
            src_reg.rsize = (reg_size)type_i32->size;
        }
        target_reg->addr = src_reg.addr;
        target_reg->rsize = src_reg.rsize;
        target_reg->type = src_reg.type;
    } else {
        if (src_reg.type == type_comptime_int
                && target_reg->type->tag == TK_FUND) {
            src_reg.type = target_reg->type;
            src_reg.rsize = target_reg->rsize;
        }
    }
    reg_typecheck(token, *target_reg, src_reg);
    emit_mov_reg(*target_reg, src_reg);
    printd("stmt:reg_assign, size %d\n", target_reg->rsize);
    cur_target->target_assigned = true;
    return true;
}

symbol_t *fn_call(parser_context *context) {
    const token_t *token = &context->cur_token;
    const str token_str = token->id;
    symbol_t *symbol = hashmap_symbol_t_tryfind(fn_ids, token_str);
    if (!symbol) {
        compile_err(token, "trying to use undefined function "), str_printerr(token->id);
        return symbol;
    }
    context->last_fn_call = symbol;

    bool multiline = token_str.end[0] == '\n';
    if (multiline) {
        return NULL;
    }

    arr_reg_t *params = &symbol->params;
    context->reg.reg_type = PARAM;
    context->reg.offset = 0;
    read_and_check_types(context, params);

    printd("found %d args\n", context->reg.offset);

    if (context->reg.offset > 0)
        lex(context);

    const str *cur_token_str = &token->id;

    if (!str_eq_lit(cur_token_str, "=>")) {
        compile_err(token, "function call '=>' is expected\n");
    }

    str fn_name = symbol->name;

    int arg_counts = context->reg.offset;
    if (do_airity_check && arg_counts != symbol->airity) {
        compile_err(token, "expected argument count %d, but found %d\n", symbol->airity, arg_counts);
    }
    emit_fn_call(&fn_name);

    context->reg.offset = 0;
    context->reg.reg_type = RET;
    reg_t *return_reg = &symbol->rets.data[0];
    type_t *return_type = return_reg->type;
    context->reg.type = return_reg->type;
    context->reg.addr = return_reg->addr;
    if (return_reg->addr)
        context->reg.rsize = sizeof (void *);
    else
        context->reg.rsize = (reg_size)return_type->size;
    context->calls_fn = true;
    return symbol;
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
    if (token->end[0] == '\n') {
        context->reg.offset = 0;
        context->reg.type = NULL;
        context->reg.rsize = 0;
    }
}

void start_of_block(parser_context *context) {
    printd("\nstart of a block\n");
    arr_mini_hashset_push(&local_ids);
    arr_u16_push(&context->deferred_unnamed_br, DEFERRED_NONE);
}

void end_of_block(parser_context *context) {
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
    TIMER_LABEL_STR(&context->name);

    TIMER_START(parse_while);
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
            printd("end of fn\n");
            break;
        }
    }
    TIMER_END(parse_while);

    TIMER_START(parse_emit);
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
    TIMER_END(parse_emit);
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
            .name = name,
        };
        type_t *t = hashmap_type_t_overwrite(types, name, &s);
        if (str_eq_lit(&name, "i32")) {
            type_i32 = t;
        }
    }
}

int main(int argc, const char *argv[]) {
    TIMER_START(clock_full);
    if (argc == 1) {
        fprintf(stderr, "usage: alc [filename]\n");
        exit(EXIT_FAILURE);
    }
    TIMER_START(clock_read_source);
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
    TIMER_END(clock_read_source);

    TIMER_START(clock_make_output_name);
    size_t source_name_len = strlen(source_name);
    char *out_name = malloc(source_name_len + 1);
    if (!out_name)
        malloc_failed();
	memset(out_name, 0, source_name_len + 1);
    strncpy(out_name, source_name, source_name_len - 1);
    out_name[source_name_len - 2] = 's';

    TIMER_END(clock_make_output_name);
    TIMER_START(clock_make_output_fopen);
    FILE *object_file = fopen(out_name, "w");
    if (object_file == NULL) {
        fprintf(stderr, "error: failed to create file\n");
        exit(EXIT_FAILURE);
    }
    TIMER_END(clock_make_output_fopen);

    iter src = { .start = source_start, .cur = source_start, .end = source_start + source_len };

    TIMER_START(clock_zero);
    TIMER_END(clock_zero);
    emit_init();
    register_fund_types();
    TIMER_START(clock_parse_all);
    while (src.cur < src.end) {
        function(&src, object_file);
    }

    emit_cstr(object_file);
    TIMER_END(clock_parse_all);

    if (has_compile_err)
        fprintf(stderr, CSI_RED"compilation failed\n"CSI_RESET);
    TIMER_END(clock_full);
    return has_compile_err;
}
