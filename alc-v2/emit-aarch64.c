#include "types.h"
#include <stdio.h>
#include <inttypes.h>
#ifndef _WIN32
#include <execinfo.h>
#include <unistd.h>
#endif

#include "buffer.h"
#include "emit.h"
#include "str.h"
#include "err.h"
#include "typesys.h"
#include "emit_helper.h"

#define INIT_BUFSIZ 0x400
#define CALLEE_START 19

#define DECL_PTR(T, X) T _##X; T *X = &_##X

#define INSTR(s) "\t"s"\n"
#define STR_FROM_INSTR(s) STR_FROM(INSTR(s))

emit_context_t *context;
static buf *fn_buf;


static buf text_buf;

static buf cstr_buf;
static char *cstr_begin;
static unsigned string_lit_counts;

extern const char *addrgen_adrp;
extern const char *addrgen_add;
extern const char *fn_prefix;
extern const char *fn_annotation_fmt;
extern const char *local_string_prefix;
extern type_t *type_comptime_int;

void str_printerr(str s);

const char *const cond_str[] = {
    "eq", "ne", "ge", "lt",
};

const size_t default_register_size = 8;

void emit_init(void) {
    buf_init(&text_buf, INIT_BUFSIZ);
    buf_puts(&text_buf, STR_FROM(text_section_header));

    buf_init(&cstr_buf, INIT_BUFSIZ);
    buf_puts(&cstr_buf, STR_FROM(string_section_header));
    cstr_begin = cstr_buf.cur;
}

#define MAX_CONTEXTS 10
emit_context_t *contexts[MAX_CONTEXTS];
emit_context_t **contexts_top = contexts;

void emit_reset_fn(emit_context_t *in_context) {
    if (contexts_top == contexts + MAX_CONTEXTS) {
        report_error("max contexts reached. #import or #compile recursion might be too deep");
        return;
    }
    *contexts_top++ = context;

    context = in_context;
    fn_buf = &context->fn_buf;
    buf_init(&context->fn_header_buf, 0x100);
    buf_init(&context->prologue_buf, INIT_BUFSIZ);
    buf_init(&context->fn_buf, INIT_BUFSIZ);
}

void emit_finalize_fnbuf(emit_context_t *emit_ctx, FILE *out) {
    if (out != NULL) {
        buf_fwrite(&emit_ctx->fn_header_buf, out);
        buf_fwrite(&emit_ctx->prologue_buf, out);
        buf_fwrite(&emit_ctx->fn_buf, out);
    }

    if (contexts_top == contexts) {
        context = NULL;
        fn_buf = NULL;
    } else {
        context = *--contexts_top;
        fn_buf = context != NULL ? &context->fn_buf : NULL;
    }
}

void emit_text(FILE *out) {
    buf_fwrite(&text_buf, out);
}

void emit_cstr(FILE *out) {
    if (cstr_begin < cstr_buf.cur) {
        buf_fwrite(&cstr_buf, out);
    }
}

bool emit_need_escaping(void) {
    return false;
}

const reg_t FP = { .reg_type = FRAME, .rsize = sizeof (void *) };

static int get_regoff(reg_t e) {
    if (e.reg_type == SCRATCH)
        e.offset += 8;
    else if (e.reg_type == NREG)
        e.offset += 19;
    else if (e.reg_type == FRAME)
        e.offset = 29;
    else if (e.reg_type == STACK)
        e.offset = 31;
    return e.offset;
}

static const char *get_wx(reg_size reg_size) {
    const char *format;
    if (reg_size <= 4) {
        format = "w";
    } else if (reg_size <= 8) {
        format = "x";
    } else {
        report_error(CSI_RED"aarch64: cannot load size bigger than 8 to register (was %d)\n"CSI_RESET, reg_size);
        format = "x";
    }
    return format;
}

void buf_putreg(buf *buffer, reg_t reg) {
    if (reg.reg_type == RD_NONE) {
        buf_snprintf(buffer, reg.rsize <= 4 ? "wzr" : "xzr");
        return;
    }
    if (reg.reg_type == STACK) {
        buf_puts(buffer, STR_FROM("sp"));
    } else {
        const char *format;
        if (reg.rsize <= 4) {
            format = "w%d";
        } else if (reg.rsize <= 8) {
            format = "x%d";
        } else {
            report_error(CSI_RED"aarch64: cannot load size bigger than 8 to register (was %d)\n"CSI_RESET, reg.rsize);
            return;
        }
        buf_snprintf(buffer, format, get_regoff(reg));
    }
}

// Pack consecutive sub-byte VALUE members into a single immediate.
// Advances *i past any members it consumes; caller's loop will re-increment.
static void pack_small_values(const dyn_member_t *members, const dyn_agg_member *args,
                               ptrdiff_t *i, ptrdiff_t member_count,
                               size_t first_size, i64 *value) {
    size_t packed = first_size;
    while (packed < 2) {
        if (++(*i) >= member_count)
            break;
        agg_member *next_r = args->begin + *i;
        if (next_r->tag != VALUE) {
            --(*i);
            break;
        }
        member_t *next = &members->begin[*i];
        size_t next_size = dtype_size(&next->type);
        if (packed + next_size > 2) {
            --(*i);
            break;
        }
        packed += next_size;
        *value |= next_r->value << (next_size * 8);
    }
}

static void emit_member_value(reg_t dst, const dyn_member_t *members, const dyn_agg_member *args,
                               ptrdiff_t *i, ptrdiff_t member_count,
                               size_t memb_size, size_t offset_bits,
                               bool *dst_initialized) {
    i64 value = args->begin[*i].value;
    if (offset_bits >= 32 && dst.rsize < 8)
        dst.rsize = 8;
    if (offset_bits % 16 == 0) {
        pack_small_values(members, args, i, member_count, memb_size, &value);
    }
    if (value == 0)
        return;

    if (offset_bits % 16 != 0) {
        if (!*dst_initialized) {
            emit_ri(STR_FROM("mov"), dst, value << offset_bits);
            unreachable;
        }
        emit_rri(STR_FROM("orr"), dst, dst, value << offset_bits);
        return;
    }
    if (!*dst_initialized) {
        emit_risi(STR_FROM("movz"), dst, value, STR_FROM("lsl"), (i64)offset_bits);
    } else {
        emit_risi(STR_FROM("movk"), dst, value, STR_FROM("lsl"), (i64)offset_bits);
    }
    *dst_initialized = true;
}

static void emit_member_reg(reg_t dst, reg_t reg, size_t memb_size, size_t offset_bits,
                             bool dst_initialized) {
    if (reg.reg_type == STACK) {
        emit_sub(dst, FP, reg.offset);
        return;
    }
    if (offset_bits == 0) {
        if (memb_size < 4) {
            int mask = 0xff;
            for (size_t j = 1; j < memb_size; ++j) {
                mask |= 0xff << (j * 8);
            }
            if (reg.rsize < dst.rsize) {
                reg.rsize = dst.rsize;
            }
            emit_rri(STR_FROM("and"), dst, reg, mask);
        } else {
            reg_t tmp_dst = dst;
            // no need to emit mov?
            if (reg.rsize < tmp_dst.rsize) {
                tmp_dst.rsize = reg.rsize;
            }
            tmp_dst.rsize = (u8)memb_size;
            emit_rr(STR_FROM("mov"), tmp_dst, reg);
        }
    } else {
        if (reg.rsize < dst.rsize) {
            reg.rsize = dst.rsize;
        }
        i64 width = (i64)memb_size * 8;
        if (dst_initialized) {
            emit_rrii(STR_FROM("bfi"),   dst, reg, (i64)offset_bits, width);
        } else {
            emit_rrii(STR_FROM("ubfiz"), dst, reg, (i64)offset_bits, width);
        }
    }
}

bool emit_eightbyte_struct(reg_t dst, const dtype_t *dtype, const dyn_agg_member *args, int *index, size_t *size) {
    const type_t *type = dtype->base;
    const dyn_member_t *members = &type->struct_t.members;
    ptrdiff_t member_count = args->cur - args->begin;
    dst.rsize = type->size > 8 ? 8 : (reg_size)type->size;

    bool is_arr = dtype_top(dtype).tag == DK_ARRAY;
    bool dst_initialized = false;
    size_t size_acc = 0;
    size_t base_offset_bits = 0;

    for (ptrdiff_t i = *index; i < member_count; ++i, *index = (int)i) {
        member_t *memb = is_arr
            ? &(member_t){.type = *dtype, .offset = (size_t)i * dtype->base->size}
            : &members->begin[i];
        size_t memb_size = is_arr ? dtype->base->size : dtype_size(&memb->type);
        size_t offset_bits = memb->offset * 8;

        if (size_acc == 0) {
            base_offset_bits = offset_bits;
        }
        offset_bits -= base_offset_bits;

        if (size_acc + memb_size > 8)
            break;
        size_acc += memb_size;

        agg_member *r = &args->begin[i];
        if (r->tag == VALUE) {
            emit_member_value(dst, members, args, &i, member_count, memb_size, offset_bits, &dst_initialized);
        } else if (r->tag == REG) {
            emit_member_reg(dst, r->reg, memb_size, offset_bits, dst_initialized);
            dst_initialized = true;
        } else {
            unreachable;
        }
    }

    *size += size_acc;
    return dst_initialized;
}

void emit_zero_out(reg_t dst) {
    emit_mov(dst, 0);
}

void emit_cond_set(reg_t dst, cond_t cond) {
    emit_rx(STR("cset"), dst);
    buf_puts(fn_buf, STR(", "));
    buf_puts(fn_buf, STR_FROM(cond_str[cond]));
    buf_putc(fn_buf, '\n');
}

static void emit_stp(reg_t src1, reg_t src2, reg_t base, i64 offset) {
    emit_rrx(STR("stp"), src1, src2);
    buf_puts(fn_buf, STR(", ["));
    buf_putreg(fn_buf, base);
    buf_snprintf(fn_buf, ", #%"PRId64"]\n", offset);
}

const reg_t xzr = {.reg_type = RD_NONE, .rsize = 8};
const reg_t wzr = {.reg_type = RD_NONE, .rsize = 4};

void emit_zerofill(reg_t dst, i64 offset, const dtype_t *type) {
    size_t size = dtype_size(type);

    while (size >= 16) {
        emit_stp(xzr, xzr, dst, offset);
        size -= 16;
        offset += 16;
    }
    while (size >= 8) {
        emit_str(dst, xzr, (int)offset);
        size -= 8;
        offset += 8;
    }

    reg_t zr = wzr;
    reg_size rsize = 4;
    while (size) {
        if (size >= rsize) {
            zr.rsize = rsize;
            emit_str(dst, zr, (int)offset);
            size -= rsize;
            offset += rsize;
        }
        rsize /= 2;
    }
}

void emit_store_eightbytes(reg_t base, i64 offset, reg_t lo, bool lo_written,
                           reg_t hi, bool hi_written, bool has_hi) {
    if (has_hi) {
        lo.rsize = 8;
        hi.rsize = 8;
        if (!lo_written) {
            lo = xzr;
        }
        if (!hi_written) {
            hi = xzr;
        }
        emit_stp(lo, hi, base, offset);
        return;
    }

    if (!lo_written) {
        lo.reg_type = RD_NONE;
    }
    emit_str(base, lo, (int)offset);
}


void emit_mov(reg_t dst, i64 value) {
    int regidx = get_regoff(dst);
    if (dst.dtype.base == type_comptime_int) {
        if (value <= INT32_MAX) {
            buf_snprintf(fn_buf, INSTR("mov %s%d, #%"PRId32), get_wx(dst.rsize), regidx, (i32)value);
        } else if (value <= INT64_MAX) {
            dst.rsize = 8;
            //emit_ri(STR("mov"), dst, value);
            buf_snprintf(fn_buf, INSTR("mov %s%d, #%"PRIu64), get_wx(dst.rsize), regidx, value);
        } else if ((u64)value <= UINT64_MAX) {
            dst.rsize = 8;
            buf_snprintf(fn_buf, INSTR("mov %s%d, #%"PRIu64), get_wx(dst.rsize), regidx, value);
        } else {
            compile_err(NULL, "literal was too big");
        }
    } else {
        if (!dst.dtype.base || !dst.dtype.base->sign)
            buf_snprintf(fn_buf, INSTR("mov %s%d, #%"PRIx64), get_wx(dst.rsize), regidx, value);
        else
            buf_snprintf(fn_buf, INSTR("mov %s%d, #%"PRIi64), get_wx(dst.rsize), regidx, value);
    }

}

void type_conv(reg_t dst, reg_t src) {
    const type_t *srct = src.dtype.base;
    if (srct == NULL) {
        report_error("compiler bug: type of the src reg was null\n");
        return;
    }

    buf_putc(fn_buf, '\t');
    if (srct->sign) {
        buf_puts(fn_buf, STR_FROM("sxt"));
        if (srct->size == 1) {
            buf_putc(fn_buf, 'b');
        } else if (srct->size == 2) {
            buf_putc(fn_buf, 'h');
        } else if (srct->size == 4) {
            buf_putc(fn_buf, 'w');
        } else {
            report_error("incorrect src size %d\n", srct->size);
        }


        buf_snprintf(fn_buf, " %s%d, ", get_wx(dst.rsize), get_regoff(dst));
        buf_snprintf(fn_buf, "%s%d\n", get_wx(src.rsize), get_regoff(src));
    } else {
        const char *dst_ws = get_wx(dst.rsize);
        if (srct->size == 1) {
            buf_snprintf(fn_buf, "and %s%d, %s%d, #0xff\n",
                    dst_ws, get_regoff(dst),
                    dst_ws, get_regoff(src));
        } else if (srct->size == 2) {
            buf_snprintf(fn_buf, "and %s%d, %s%d, #0xffff\n",
                    dst_ws, get_regoff(dst),
                    dst_ws, get_regoff(src));
        } else if (srct->size == 4) {
            buf_snprintf(fn_buf, "mov w%d, w%d\n", get_regoff(dst), get_regoff(src));
        }
    }
}

void emit_mov_reg(reg_t dst, reg_t src) {
    const char *format;
    if (dtype_tryget_addr(&dst.dtype)) {
        dst.rsize = 8;
    }
    if (dst.rsize > src.rsize) {
        type_conv(dst, src);
        return;
    }
    if (dst.rsize <= 4) {
        format = INSTR("mov w%d, w%d");
    } else if (dst.rsize <= 8) {
        format = INSTR("mov x%d, x%d");
    } else {
        unreachable;
    }
    buf_snprintf(fn_buf, format, get_regoff(dst), get_regoff(src));
}

void emit_add(reg_t dst, reg_t lhs, i64 rhs) {
    buf_puts(fn_buf, STR_FROM("\tadd "));
    buf_putreg(fn_buf, dst);
    buf_puts(fn_buf, STR_FROM(", "));
    buf_putreg(fn_buf, lhs);
    buf_puts(fn_buf, STR_FROM(", "));
    buf_snprintf(fn_buf, "#%"PRId64"\n", rhs);
}

void emit_add_reg(reg_t dst, reg_t lhs, reg_t rhs) {
    buf_puts(fn_buf, STR_FROM("\tadd "));
    buf_putreg(fn_buf, dst);
    buf_puts(fn_buf, STR_FROM(", "));
    buf_putreg(fn_buf, lhs);
    buf_puts(fn_buf, STR_FROM(", "));
    buf_putreg(fn_buf, rhs);
    buf_putc(fn_buf, '\n');
}

void emit_sub(reg_t dst, reg_t lhs, i64 rhs) {
    emit_rri(STR_FROM("sub"), dst, lhs, rhs);
}
void emit_sub_reg(reg_t dst, reg_t lhs, reg_t rhs) {
    emit_rrr(STR_FROM("sub"), dst, lhs, rhs);
}

void emit_cmp(reg_t lhs, i64 rhs) {
    buf_snprintf(fn_buf, INSTR("cmp w%d, #%"PRId64),
            get_regoff(lhs), rhs);
}

void emit_cmp_reg(reg_t lhs, reg_t rhs) {
    buf_snprintf(fn_buf, INSTR("cmp w%d, w%d"),
            get_regoff(lhs), get_regoff(rhs));
}

void emit_string_lit(reg_t dst, const str *s) {
    char *buffer = malloc(SPRINTF_BUFSIZ);
    if (!buffer)
        malloc_failed();
    int num_printed = snprintf(buffer, SPRINTF_BUFSIZ, local_string_prefix, string_lit_counts++);
    if (num_printed >= SPRINTF_BUFSIZ) {
        fputs("buffer overflow in snprintf\n", stderr);
        exit(EXIT_FAILURE);
    }
    int regidx = get_regoff(dst);
    buf_snprintf(fn_buf, addrgen_adrp, regidx, buffer);
    buf_snprintf(fn_buf, addrgen_add, regidx, regidx, buffer);

    buf_snprintf(&cstr_buf, "%s:\n", buffer);
    buf_puts(&cstr_buf, STR_FROM("\t.asciz "));
    buf_puts(&cstr_buf, *s);
    buf_putc(&cstr_buf, '\n');

    free(buffer);
}

void emit_lsl(reg_t dst, reg_t lhs, i64 rhs) {
    emit_rri(STR_FROM("lsl"), dst, lhs, rhs);
}

static void load_store_x(const char *op, reg_t r0, reg_t r1) {
    const char *suffix = "";
    size_t size = r0.rsize;
    if (dtype_tryget_addr(&r0.dtype)) {
        size = 8;
    }
    if (r0.rsize <= 0) {
        compile_err(NULL, "cannot %s size of zero\n", op);

        printd("dump r0 | size: %d, reg_type: %d, offset: %d\n", r0.rsize, r0.reg_type, r0.offset);
        report_error("");
    } else if (size <= 1) {
        if (*op == 'l' && r0.dtype.base->sign)
            suffix = "sb";
        else
            suffix = "b";
    } else if (size <= 2) {
        if (*op == 'l' && r0.dtype.base->sign)
            suffix = "sh";
        else
            suffix = "h";
    } else if (*op == 'l' && size == 8 && size == 4) {
        if (*op == 'l' && r0.dtype.base->sign)
            suffix = "sw";
    }
    buf_snprintf(fn_buf, ("\t%s%s "), op, suffix);
    buf_putreg(fn_buf, r0);
    buf_snprintf(fn_buf, ", [");
    buf_putreg(fn_buf, r1);
    buf_snprintf(fn_buf, ", ");
}

void emit_str(reg_t dst, reg_t src, int offset) {
    load_store_x("str", src, dst);
    buf_snprintf(fn_buf, ("#%d]\n"), offset);
}

void emit_ldr(reg_t dst, reg_t src, int offset) {
    load_store_x("ldr", dst, src);
    buf_snprintf(fn_buf, ("#%d]\n"), offset);
}

void emit_str_reg(reg_t dst, reg_t src, reg_t offset) {
    load_store_x("str", src, dst);
    buf_snprintf(fn_buf, ("x%d]\n"), get_regoff(offset));
}

void str_lsl(reg_t dst, reg_t src, reg_t offset, int lsl) {
    load_store_x("str", src, dst);
    buf_snprintf(fn_buf, ("x%d, lsl #%d]\n"), get_regoff(offset), lsl);
}

void ldr_lsl(reg_t dst, reg_t src, reg_t offset, int lsl) {
    load_store_x("ldr", dst, src);
    buf_snprintf(fn_buf, ("x%d, lsl #%d]\n"), get_regoff(offset), lsl);
}

void emit_array_access(reg_t dst, reg_t src, reg_t offset, load_store_t is_store) {
    dtype_t *dtype = &src.dtype;
    size_t array_size = dtype_size(dtype);
    size_t elem_size = dtype->base->size;

    if (src.reg_type == STACK) {
        if (src.offset) {
            reg_t tmp_src = src;
            tmp_src.reg_type = SCRATCH;
            tmp_src.offset = 3;
            tmp_src.rsize = 8;
            dtype_push(&tmp_src.dtype, (declarator_t){.tag = DK_ADDR, .amount = 1});
            emit_sub(tmp_src, FP, src.offset);
            src = tmp_src;
        }
    }

    if (elem_size == 0) {
        compile_err(NULL, "element size was zero\n");
        return;
    }
    if (elem_size == 1) {
        if (is_store)
            emit_str_reg(dst, src, offset);
        else
            emit_ldr_reg(dst, src, offset);
        return;
    }
    if (elem_size <= 8) {
        int exp = power_of_two_exponent(elem_size);
        if (exp) {
            if (is_store)
                str_lsl(dst, src, offset, exp);
            else
                ldr_lsl(dst, src, offset, exp);
            return;
        }
    }

    reg_t size;
    if (array_size) {
        size = (reg_t){.reg_type = SCRATCH, .offset = 1};
        emit_mov(size, (i64)array_size);
    } else {
        size = (reg_t){.reg_type = RD_NONE,};
    }


    reg_t index = {.reg_type = SCRATCH, .offset = 2};
    buf_snprintf(fn_buf, ("\tsmull x%d, w%d, "),
            get_regoff(index), get_regoff(index));
    buf_putreg(fn_buf, size);
    buf_snprintf(fn_buf, "\n");

    if (is_store)
        emit_str_reg(dst, src, offset);
    else
        emit_ldr_reg(dst, src, offset);
}

void emit_ldr_reg(reg_t dst, reg_t src, reg_t offset) {
    load_store_x("ldr", dst, src);
    buf_snprintf(fn_buf, ("x%d]\n"), get_regoff(offset));
}

void emit_branch(str fn_name, str label, int index) {
    buf_puts(fn_buf, STR_FROM("\tb "));
    put_label(fn_name, label, index);
    buf_puts(fn_buf, STR_FROM("\n"));
}

bool emit_branch_cond(cond_t condition, str fn_name, str label, int index) {
    buf_puts(fn_buf, STR_FROM("\tb."));
    if (condition >= (cond_t)(sizeof (cond_str) / sizeof cond_str[0])) {
        fprintf(stderr, CSI_RED"unknown condition %d\n", condition);
        return false;
    }
    buf_puts(fn_buf, STR_FROM(cond_str[condition]));
    buf_putc(fn_buf, ' ');
    put_label(fn_name, label, index);
    buf_puts(fn_buf, STR_FROM("\n"));
    return true;
}

void emit_label(str fn_name, str label, int index) {
    put_label(fn_name, label, index);
    buf_puts(fn_buf, STR_FROM(":\n"));
}

void emit_fn_prologue_epilogue(const parser_context *parser_context) {
    context->prologue_buf.cur = context->prologue_buf.start;
    if (!parser_context->calls_fn
            && parser_context->max_nreg_count == 0
            && parser_context->stack_size == 0)
        return;

    int regs_to_save = parser_context->max_nreg_count;
    if (regs_to_save + CALLEE_START >= 28) {
        compile_err(&parser_context->cur_token, "used up all callee-saved registers. found %d (expected less than %d", parser_context->max_nreg_count, 28 - CALLEE_START);
        return;
    }
    bool calls_fn = parser_context->calls_fn;
    if (calls_fn) {
        regs_to_save += 2;
    }

    int stack_size =
        ALIGN_TO(regs_to_save * (signed)sizeof (u64), 16)
        + ALIGN_TO(parser_context->stack_size, 16);
    printd("\nstack report for: ");
    str_printd(parser_context->name);
    printd("\t- regs to save: %d, stack size: %d\n", regs_to_save, parser_context->stack_size);
    printd("\t- aligned regs: %d, aligned stack: %d\n",
            ALIGN_TO(regs_to_save * (signed)sizeof (u64), 16),
            ALIGN_TO(parser_context->stack_size, 16));
    printd("\t- result stack: %d\n", stack_size);

    int cur_stackoff = ALIGN_TO(parser_context->stack_size, 16);
    const int stack_objs_size = cur_stackoff;
    if (stack_objs_size > 0) {
        buf_snprintf(&context->prologue_buf, INSTR("sub sp, sp, #%d"), stack_size);
    }

    int remaining = regs_to_save;
    bool defer_ldp = false;
    int deferred0 = 0, deferred1 = 0;

    if (remaining > 1) {
        int reg0 = CALLEE_START + remaining - 1;
        int reg1 = reg0 - 1;
        int off = cur_stackoff;
        if (calls_fn) {
            reg0 = 29, reg1 = 30;
        }
        const char *stp_format = INSTR("stp x%d, x%d, [sp, #%d]");
        const char *ldp_format = INSTR("ldp x%d, x%d, [sp, #%d]");
        if (cur_stackoff == 0) {
            stp_format = INSTR("stp x%d, x%d, [sp, #-%d]!");
            defer_ldp = true;
            off = stack_size;
        }
        buf_snprintf(&context->prologue_buf, stp_format, reg0, reg1, off);
        if (!defer_ldp)
            buf_snprintf(fn_buf, ldp_format, reg0, reg1, off);
        else
            deferred0 = reg0, deferred1 = reg1;
        remaining -= 2;
        cur_stackoff += 16;
    }
    while (remaining > 1) {
        int reg0 = CALLEE_START + remaining - 1;
        int reg1 = reg0 - 1;
        buf_snprintf(&context->prologue_buf, INSTR("stp x%d, x%d, [sp, #%d]"),
                reg0, reg1, cur_stackoff);
        buf_snprintf(fn_buf, INSTR("ldp x%d, x%d, [sp, #%d]"),
                reg0, reg1, cur_stackoff);
        remaining -= 2;
        cur_stackoff += 16;
    }
    if (remaining == 1) {
        remaining -= 1;
        const char *stp_format = INSTR("str x%d, [sp, #%d]");
        const char *ldp_format = INSTR("ldr x%d, [sp, #%d]");
        int off = cur_stackoff;
        if (cur_stackoff == 0) {
            stp_format = INSTR("str x%d, [sp, #%d]!");
            ldp_format = INSTR("ldr x%d, [sp], #%d");
            off = stack_size;
        }
        int reg0 = CALLEE_START + remaining;
        buf_snprintf(&context->prologue_buf, stp_format,
                reg0, off);
        buf_snprintf(&context->fn_buf, ldp_format,
                reg0, off);
        cur_stackoff += 16;
    }

    if (stack_objs_size == 0) {
        buf_puts(&context->prologue_buf, STR_FROM_INSTR("mov x29, sp"));
    } else {
        buf_snprintf(&context->prologue_buf, INSTR("add x29, sp, #%d"), stack_objs_size);
    }

    if (defer_ldp) {
        const char *ldp_format = INSTR("ldp x%d, x%d, [sp], #%d");
        buf_snprintf(&context->fn_buf, ldp_format, deferred0, deferred1, stack_size);
    }
    if (stack_objs_size > 0) {
        buf_snprintf(&context->fn_buf, INSTR("add sp, sp, #%d"), stack_size);
    }
}

void emit_fn_call(const str *s) {
    buf_puts(&context->fn_buf, STR_FROM("\tbl "));
    buf_puts(&context->fn_buf, STR_FROM(fn_prefix));
    buf_puts(&context->fn_buf, *s);
    buf_putc(&context->fn_buf, '\n');
}

void emit_fn(str fn_name) {
    buf_puts(&context->fn_header_buf, STR_FROM("\n\t.globl "));
    buf_puts(&context->fn_header_buf, STR_FROM(fn_prefix));
	buf_puts(&context->fn_header_buf, fn_name);
    buf_puts(&context->fn_header_buf, STR_FROM("\n\t.p2align 2\n"));
    if (*fn_annotation_fmt) {
        buf_snprintf(&context->fn_header_buf, fn_annotation_fmt,
                     (int)str_len(fn_name), fn_name.data);
    }
    buf_puts(&context->fn_header_buf, STR_FROM(fn_prefix));
	buf_puts(&context->fn_header_buf, fn_name);
    buf_puts(&context->fn_header_buf, STR_FROM(":\n"));
}

void emit_ret(void) {
    buf_puts(&context->fn_buf, STR_FROM_INSTR("ret"));
}

#ifndef _WIN32
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 1, 2)))
#endif
void report_error(const char *format, ...) {
    int size = 0x1000;
    void *array[size];
    size = backtrace(array, size);

    va_list args;
    va_start(args, format);
    fprintf(stderr, CSI_RED"error: "CSI_RESET);
    vfprintf(stderr, format, args);
    va_end(args);
    compile_err(NULL, "");

    backtrace_symbols_fd(array, size, STDERR_FILENO);
}
#else
void report_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, CSI_RED"error: "CSI_RESET);
    vfprintf(stderr, format, args);
    va_end(args);
    compile_err(NULL, "");
}
#endif
