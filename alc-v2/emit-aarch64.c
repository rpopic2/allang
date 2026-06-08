#include "types.h"
#include <stdio.h>
#include <inttypes.h>

#include "buffer.h"
#include "emit.h"
#include "err.h"
#include "str.h"
#include "typesys.h"
#include "emit_helper.h"

#define CALLEE_START 19

extern const char *addrgen_adrp;
extern const char *addrgen_add;
extern const char *fn_prefix;
extern const char *fn_annotation_fmt;
extern const char *local_string_prefix;
extern type_t *type_comptime_int;

const char *error_too_big = CSI_RED"aarch64: cannot load size bigger than 8 to register (was %d)\n"CSI_RESET;

const char *imm_prefix = "#";

// see enum cond
const char *const cond_str[] = {
    "eq", "ne", "ge", "lt", "gt", "le", "hs", "lo", "hi", "ls",
};

const size_t default_register_size = 8;

const reg_t FP = { .reg_type = FRAME, .rsize = sizeof (void *) };

void str_printerr(str s);

bool emit_need_escaping(void) {
    return false;
}

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
        report_error(error_too_big, reg_size);
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
        buf_puts(buffer, STR("sp"));
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
static size_t pack_small_values(const dyn_member_t *members, const dyn_agg_member *args,
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
        size_t next_size = members ? dtype_size(&members->begin[*i].dtype) : first_size;
        if (packed + next_size > 2) {
            --(*i);
            break;
        }
        packed += next_size;
        *value |= next_r->value << (next_size * 8);
    }
    return packed;
}

static void emit_mov_lane(reg_t dst, i64 chunk, i64 offset_bits, bool *initialized) {
    if (*initialized) {
        emit_risi(STR("movk"), dst, chunk, STR("lsl"), offset_bits);
    } else {
        emit_risi(STR("movz"), dst, chunk, STR("lsl"), offset_bits);
        *initialized = true;
    }
}

static size_t emit_member_value(reg_t dst, const dyn_member_t *members, const dyn_agg_member *args,
                                ptrdiff_t *i, ptrdiff_t member_count,
                                size_t memb_size, size_t offset_bits,
                                bool *dst_initialized) {
    i64 value = args->begin[*i].value;
    size_t packed = memb_size;
    if (offset_bits >= 32 && dst.rsize < 8)
        dst.rsize = 8;
    if (offset_bits % 16 == 0) {
        packed = pack_small_values(members, args, i, member_count, memb_size, &value);
    }
    if (value == 0)
        return packed;

    if (offset_bits % 16 != 0) {
        if (!*dst_initialized) {
            emit_ri(STR("mov"), dst, value << offset_bits);
            unreachable;
        }
        emit_rri(STR("orr"), dst, dst, value << offset_bits);
        return packed;
    }
    emit_mov_lane(dst, value, (i64)offset_bits, dst_initialized);
    return packed;
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
            emit_rri(STR("and"), dst, reg, mask);
        } else {
            reg_t tmp_dst = dst;
            // no need to emit mov?
            if (reg.rsize < tmp_dst.rsize) {
                tmp_dst.rsize = reg.rsize;
            }
            tmp_dst.rsize = (u8)memb_size;
            emit_rr(STR("mov"), tmp_dst, reg);
        }
    } else {
        if (reg.rsize < dst.rsize) {
            reg.rsize = dst.rsize;
        }
        i64 width = (i64)memb_size * 8;
        if (dst_initialized) {
            emit_rrii(STR("bfi"),   dst, reg, (i64)offset_bits, width);
        } else {
            emit_rrii(STR("ubfiz"), dst, reg, (i64)offset_bits, width);
        }
    }
}

bool emit_eightbyte_struct(reg_t dst, const dtype_t *dtype, const dyn_agg_member *args, int *index, size_t *size, size_t limit) {
    const type_t *type = dtype->base;
    ptrdiff_t member_count = args->cur - args->begin;
    dst.rsize = type->size > 8 ? 8 : (reg_size)type->size;

    bool is_arr = dtype_top(dtype).tag == DK_ARRAY;
    const dyn_member_t *members = is_arr ? NULL : &type->struct_t.members;
    bool dst_initialized = false;
    size_t size_acc = 0;
    size_t base_offset_bits = 0;

    for (ptrdiff_t i = *index; i < member_count; ++i, *index = (int)i) {
        member_t *memb = is_arr
            ? &(member_t){.dtype = *dtype, .offset = (size_t)i * dtype->base->size}
            : &members->begin[i];
        size_t memb_size = is_arr ? dtype->base->size : dtype_size(&memb->dtype);
        size_t offset_bits = memb->offset * 8;

        if (size_acc == 0) {
            base_offset_bits = offset_bits;
        }
        offset_bits -= base_offset_bits;

        if (size_acc + memb_size > limit)
            break;

        agg_member *r = &args->begin[i];
        if (r->tag == VALUE) {
            size_acc += emit_member_value(dst, members, args, &i, member_count, memb_size, offset_bits, &dst_initialized);
        } else if (r->tag == REG) {
            emit_member_reg(dst, r->reg, memb_size, offset_bits, dst_initialized);
            dst_initialized = true;
            size_acc += memb_size;
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
    buf_puts(fn_buf, STR(cond_str[cond]));
    buf_putc(fn_buf, '\n');
}

static void emit_stp(reg_t src1, reg_t src2, reg_t base, i64 offset) {
    emit_rrx(STR("stp"), src1, src2);
    buf_puts(fn_buf, STR(", ["));
    buf_putreg(fn_buf, base);
    buf_comma(fn_buf);
    buf_puti(fn_buf, offset);
    buf_snprintf(fn_buf, "]\n");
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
        emit_str_reg(dst, xzr, (int)offset);
        size -= 8;
        offset += 8;
    }

    reg_t zr = wzr;
    reg_size rsize = 4;
    while (size) {
        if (size >= rsize) {
            zr.rsize = rsize;
            emit_str_reg(dst, zr, (int)offset);
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
    emit_str_reg(base, lo, (int)offset);
}

void emit_store_packed(reg_t base, i64 offset, reg_t src, size_t nbytes) {
    size_t pos = 0;
    while (nbytes > 0) {
        reg_size chunk = nbytes >= 8 ? 8 : nbytes >= 4 ? 4 : nbytes >= 2 ? 2 : 1;
        reg_t piece = src;
        piece.rsize = chunk;
        emit_str_reg(base, piece, (int)(offset + (i64)pos));
        nbytes -= chunk;
        pos += chunk;
        if (nbytes > 0) {
            reg_t full = src;
            full.rsize = 8;
            emit_rri(STR("lsr"), full, full, (i64)chunk * 8);
        }
    }
}


static void emit_mov_wide(reg_t dst, i64 value) {
    if (value > UINT32_MAX) {
        dst.rsize = 8;
    }
    const int lanes = dst.rsize > 4 ? 4 : 2;
    bool initialized = false;
    for (int lane = 0; lane < lanes; lane++) {
        const i64 chunk = (value >> (lane * 16)) & 0xFFFF;
        if (chunk == 0) {
            continue;
        }
        emit_mov_lane(dst, chunk, (i64)(lane * 16), &initialized);
    }
    if (!initialized) {
        emit_mov_lane(dst, 0, 0, &initialized);
    }
}

void emit_mov(reg_t dst, i64 value) {
    int regidx = get_regoff(dst);
    if (dst.dtype.base == type_comptime_int) {
        if (value <= UINT16_MAX) {
            buf_snprintf(fn_buf, INSTR("mov %s%d, #%"PRId32), get_wx(dst.rsize), regidx, (i32)value);
        } else {
            emit_mov_wide(dst, value);
        }
    } else {
        emit_ri(STR("mov"), dst, value);
    }

}

void put_xt(const type_t *type) {
    if (type->sign) {
        buf_putc(fn_buf, 's');
    } else {
        buf_putc(fn_buf, 'u');
    }

    buf_puts(fn_buf, STR("xt"));

    if (type->size == 1) {
        buf_putc(fn_buf, 'b');
    } else if (type->size == 2) {
        buf_putc(fn_buf, 'h');
    } else if (type->size == 4) {
        buf_putc(fn_buf, 'w');
    } else {
        report_error("incorrect src size %zd\n", type->size);
    }
    buf_putc(fn_buf, ' ');
}

void type_conv(reg_t dst, reg_t src) {
    const type_t *srct = src.dtype.base;
    if (srct == NULL) {
        report_error("compiler bug: type of the src reg was null\n");
        return;
    }

    buf_putc(fn_buf, '\t');
    if (srct->sign) {
        put_xt(srct);

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
        } else {
            report_error("compiler_bug: use mov instead of type_conv\n");
        }
    }
}

void emit_mov_reg(reg_t dst, reg_t src) {
    const char *format;
    if (reg_eq(dst, src))
        return;
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
    if (rhs == 0) {
        emit_mov_reg(dst, lhs);
        return;
    }
    buf_puts(fn_buf, STR("\tadd "));
    buf_putreg(fn_buf, dst);
    buf_puts(fn_buf, STR(", "));
    buf_putreg(fn_buf, lhs);
    buf_puts(fn_buf, STR(", "));
    buf_snprintf(fn_buf, "#%"PRId64"\n", rhs);
}

static void buf_put_ext(buf *buffer, reg_t narrow) {
    const type_t *const t = narrow.dtype.base;
    buf_putc(buffer, (t && t->sign) ? 's' : 'u');
    buf_puts(buffer, STR("xt"));
    const size_t size = t ? t->size : narrow.rsize;
    buf_putc(buffer, size == 1 ? 'b' : size == 2 ? 'h' : 'w');
}

void emit_add_reg(reg_t dst, reg_t lhs, reg_t rhs) {
    if (lhs.rsize != rhs.rsize) {
        const reg_t wide   = lhs.rsize > rhs.rsize ? lhs : rhs;
        const reg_t narrow = lhs.rsize > rhs.rsize ? rhs : lhs;
        emit_rrrx(STR("add"), dst, wide, narrow);
        buf_comma(fn_buf);
        buf_put_ext(fn_buf, narrow);
        buf_putc(fn_buf, '\n');
        return;
    }
    emit_rrr(STR("add"), dst, lhs, rhs);
}

void emit_sub(reg_t dst, reg_t lhs, i64 rhs) {
    if (rhs == 0) {
        emit_mov_reg(dst, lhs);
        return;
    }
    emit_rri(STR("sub"), dst, lhs, rhs);
}
void emit_sub_reg(reg_t dst, reg_t lhs, reg_t rhs) {
    if (lhs.rsize == rhs.rsize) {
        emit_rrr(STR("sub"), dst, lhs, rhs);
        return;
    }
    if (rhs.rsize < lhs.rsize) {
        emit_rrrx(STR("sub"), dst, lhs, rhs);
        buf_comma(fn_buf);
        buf_put_ext(fn_buf, rhs);
        buf_putc(fn_buf, '\n');
        return;
    }
    const reg_t tmp = {.reg_type = SCRATCH, .offset = 8, .rsize = dst.rsize, .dtype = lhs.dtype};
    emit_mov_reg(tmp, lhs);
    emit_rrr(STR("sub"), dst, tmp, rhs);
}

void emit_cmp(reg_t lhs, i64 rhs) {
    emit_ri(STR("cmp"), lhs, rhs);
}

void emit_cmp_reg(reg_t lhs, reg_t rhs, cond_t cond) {
    if (lhs.rsize == rhs.rsize) {
        emit_rr(STR("cmp"), lhs, rhs);
        return;
    }
    if (rhs.rsize < lhs.rsize) {
        emit_rrx(STR("cmp"), lhs, rhs);
        buf_comma(fn_buf);
        buf_put_ext(fn_buf, rhs);
        buf_putc(fn_buf, '\n');
        return;
    }
    reg_t tmp = {.reg_type = SCRATCH, .offset = 8, .rsize = rhs.rsize, .dtype = lhs.dtype};
    if (cond == COND_EQ || cond == COND_NE) {
        tmp.reg_type = lhs.reg_type;
        tmp.offset = lhs.offset;
    } else {
        emit_mov_reg(tmp, lhs);
    }
    emit_rr(STR("cmp"), tmp, rhs);
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
    buf_puts(&cstr_buf, STR("\t.asciz "));
    buf_puts(&cstr_buf, *s);
    buf_putc(&cstr_buf, '\n');

    free(buffer);
}

void emit_lsl(reg_t dst, reg_t lhs, i64 rhs) {
    emit_rri(STR("lsl"), dst, lhs, rhs);
}

static void load_store_x(const char *op, reg_t r0, reg_t r1) {
    const char *suffix = "";
    size_t size = r0.rsize;
    if (dtype_tryget_addr(&r0.dtype)) {
        size = 8;
    }
    if (r0.rsize <= 0) {
        report_error(NULL, "cannot %s size of zero\n", op);
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
    r1.rsize = 8;
    buf_putreg(fn_buf, r1);
    buf_snprintf(fn_buf, ", ");
}

void emit_str_reg(reg_t dst, reg_t src, int offset) {
    load_store_x("str", src, dst);
    buf_puti(fn_buf, offset);
    buf_snprintf(fn_buf, "]\n");
}

void emit_ldr(reg_t dst, reg_t src, int offset) {
    load_store_x("ldr", dst, src);
    buf_puti(fn_buf, offset);
    buf_snprintf(fn_buf, "]\n");
}

void emit_str_regoff(reg_t dst, reg_t src, reg_t offset) {
    load_store_x("str", src, dst);
    buf_snprintf(fn_buf, ("x%d]\n"), get_regoff(offset));
}

static reg_t imm_to_reg(reg_t dst, i64 value) {
    reg_t tmp = {
        .reg_type = SCRATCH, .offset = 8,
        .rsize = (reg_size)dst.dtype.base->size,
        .dtype = {.base = dst.dtype.base},
    };
    emit_mov(tmp, value);
    return tmp;
}

void emit_str_imm(reg_t dst, i64 value, int offset) {
    emit_str_reg(dst, imm_to_reg(dst, value), offset);
}

void emit_str_imm_regoff(reg_t dst, i64 value, reg_t offset) {
    emit_str_regoff(dst, imm_to_reg(dst, value), offset);
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
        report_error("element size was zero\n");
        return;
    }
    if (elem_size == 1) {
        if (is_store)
            emit_str_regoff(dst, src, offset);
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
        emit_str_regoff(dst, src, offset);
    else
        emit_ldr_reg(dst, src, offset);
}

void emit_elem_addr(reg_t dst, reg_t object, reg_t index) {
    reg_t base = {.reg_type = SCRATCH, .offset = 0, .rsize = sizeof (void *)};
    if (object.reg_type == STACK)
        emit_sub(base, FP, object.offset);
    else
        base = object;

    const size_t elem_size = object.dtype.base->size;
    const i64 shift = __builtin_ctz((unsigned)elem_size);
    const type_t *itype = index.dtype.base;

    if (itype->size >= 8) {
        index.rsize = 8;
        emit_rrrx(STR("add"), dst, base, index);
        buf_comma(fn_buf);
        buf_puts(fn_buf, STR("lsl "));
        buf_puti(fn_buf, shift);
        buf_putc(fn_buf, '\n');
    } else {
        index.rsize = 4;
        emit_rrrx(STR("add"), dst, base, index);
        buf_comma(fn_buf);
        put_xt(itype);
        buf_puti(fn_buf, shift);
        buf_putc(fn_buf, '\n');
    }
}

void emit_ldr_reg(reg_t dst, reg_t src, reg_t offset) {
    if (src.rsize > MAX_REG_SIZE) {
        report_error("%s", error_too_big);
        return;
    }
    if (src.rsize > 8) {
        reg_t dst2 = dst;
        dst2.offset += 1;
        dtype_t popped = dtype_pop_dup(&src.dtype);
        size_t stride = dtype_size(&popped);
        pd(stride)

        reg_t scratch = {.reg_type = SCRATCH, .rsize = sizeof (void *)};
        src.rsize = sizeof (void *);
        emit_elem_addr(scratch, src, offset);

        buf_snprintf(fn_buf, INSTR("ldp x%d, x%d, [x%d]"), get_regoff(dst), get_regoff(dst2), get_regoff(scratch));
        return;
    }

    load_store_x("ldr", dst, src);
    buf_snprintf(fn_buf, ("x%d]\n"), get_regoff(offset));
}

void emit_branch(str fn_name, str label, int index) {
    buf_puts(fn_buf, STR("\tb "));
    put_label(fn_name, label, index);
    buf_puts(fn_buf, STR("\n"));
}

bool emit_branch_cond(cond_t condition, str fn_name, str label, int index) {
    buf_puts(fn_buf, STR("\tb."));
    if (condition >= (cond_t)(sizeof (cond_str) / sizeof cond_str[0])) {
        fprintf(stderr, CSI_RED"unknown condition %d\n", condition);
        return false;
    }
    buf_puts(fn_buf, STR(cond_str[condition]));
    buf_putc(fn_buf, ' ');
    put_label(fn_name, label, index);
    buf_puts(fn_buf, STR("\n"));
    return true;
}

void emit_label(str fn_name, str label, int index) {
    put_label(fn_name, label, index);
    buf_puts(fn_buf, STR(":\n"));
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
    bool needs_fp = calls_fn || parser_context->stack_size > 0;
    if (needs_fp) {
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
        buf_snprintf(&context->prologue_buf, INSTR("sub sp, sp, #0x%x"), stack_size);
    }

    int remaining = regs_to_save;
    bool defer_ldp = false;
    int deferred0 = 0, deferred1 = 0;

    if (remaining > 1) {
        int reg0 = CALLEE_START + remaining - 1;
        int reg1 = reg0 - 1;
        int off = cur_stackoff;
        if (needs_fp) {
            reg0 = 29, reg1 = 30;
        }
        const char *stp_format = INSTR("stp x%d, x%d, [sp, #0x%x]");
        const char *ldp_format = INSTR("ldp x%d, x%d, [sp, #0x%x]");
        if (cur_stackoff == 0) {
            stp_format = INSTR("stp x%d, x%d, [sp, #-0x%x]!");
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
        buf_snprintf(&context->prologue_buf, INSTR("stp x%d, x%d, [sp, #0x%x]"),
                reg0, reg1, cur_stackoff);
        buf_snprintf(fn_buf, INSTR("ldp x%d, x%d, [sp, #0x%x]"),
                reg0, reg1, cur_stackoff);
        remaining -= 2;
        cur_stackoff += 16;
    }
    if (remaining == 1) {
        remaining -= 1;
        const char *stp_format = INSTR("str x%d, [sp, #0x%x]");
        const char *ldp_format = INSTR("ldr x%d, [sp, #0x%x]");
        int off = cur_stackoff;
        if (cur_stackoff == 0) {
            stp_format = INSTR("str x%d, [sp, #-0x%x]!");
            ldp_format = INSTR("ldr x%d, [sp], #0x%x");
            off = stack_size;
        }
        int reg0 = CALLEE_START + remaining;
        buf_snprintf(&context->prologue_buf, stp_format,
                reg0, off);
        buf_snprintf(&context->fn_buf, ldp_format,
                reg0, off);
        cur_stackoff += 16;
    }

    if (needs_fp) {
        if (stack_objs_size == 0) {
            buf_puts(&context->prologue_buf, STR_FROM_INSTR("mov x29, sp"));
        } else {
            buf_snprintf(&context->prologue_buf, INSTR("add x29, sp, #0x%x"), stack_objs_size);
        }
    }

    if (defer_ldp) {
        const char *ldp_format = INSTR("ldp x%d, x%d, [sp], #0x%x");
        buf_snprintf(&context->fn_buf, ldp_format, deferred0, deferred1, stack_size);
    }
    if (stack_objs_size > 0) {
        buf_snprintf(&context->fn_buf, INSTR("add sp, sp, #0x%x"), stack_size);
    }
}

void emit_fn_call(const str *s) {
    buf_puts(&context->fn_buf, STR("\tbl "));
    buf_puts(&context->fn_buf, STR(fn_prefix));
    buf_puts(&context->fn_buf, *s);
    buf_putc(&context->fn_buf, '\n');
}

void emit_fn(str fn_name) {
    buf_puts(&context->fn_header_buf, STR("\n\t.globl "));
    buf_puts(&context->fn_header_buf, STR(fn_prefix));
	buf_puts(&context->fn_header_buf, fn_name);
    buf_puts(&context->fn_header_buf, STR("\n\t.p2align 2\n"));
    if (*fn_annotation_fmt) {
        buf_snprintf(&context->fn_header_buf, fn_annotation_fmt,
                     (int)str_len(fn_name), fn_name.data);
    }
    buf_puts(&context->fn_header_buf, STR(fn_prefix));
	buf_puts(&context->fn_header_buf, fn_name);
    buf_puts(&context->fn_header_buf, STR(":\n"));
}

void emit_ret(void) {
    buf_puts(&context->fn_buf, STR_FROM_INSTR("ret"));
}
