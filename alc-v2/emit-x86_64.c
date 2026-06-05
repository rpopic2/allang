#include <inttypes.h>

#include "emit.h"
#include "typesys.h"
#include "buffer.h"
#include "emit_helper.h"

extern const char * const rname_scratch[];
extern const char * const rname_callee[];
extern const char * const rname_param[];
extern const char * const rname_ret[];
extern const size_t rname_scratch_len;
extern const size_t rname_callee_len;
extern const size_t rname_param_len;
extern const size_t rname_ret_len;

extern const char *fn_prefix;
extern const char *fn_annotation_fmt;
extern const char *local_string_prefix;

extern const char *text_section_header;
extern const char *string_section_header;

const char *imm_prefix = "";

const size_t default_register_size = 8;

bool emit_need_escaping(void) {
    return false;
}

static void buf_putreg(buf *buffer, reg_t reg) {
    register_dst reg_type = reg.reg_type;
    if (reg_type == STACK) {
        buf_puts(buffer, STR("rsp"));
        return;
    } else if (reg_type == FRAME) {
        buf_puts(buffer, STR("rbp"));
        return;
    }
    if (reg.offset < 0) {
        report_error("unexpected negative register offset %d", reg.offset);
        return;
    }
    size_t offset = (size_t)reg.offset;

    const char *rname_original;
#define RNAME_START if (false)
#define RNAME(RTYPE, rtype) \
    } else if (reg_type == RTYPE) { \
        if (offset >= rname_##rtype##_len) { \
            report_error("used up all "#rtype" registers. offset was %zd\n", offset); \
            return; \
        } \
        rname_original = rname_##rtype[offset];

    RNAME_START {
        RNAME(SCRATCH, scratch)
        RNAME(NREG, callee)
        RNAME(PARAM, param)
        RNAME(RET, ret)
    } else unreachable;
#undef RNAME
#undef RNAME_START

    size_t rname_len = strlen(rname_original);
    char rname_arr[8] = {0};
    char *rname = rname_arr + 1;
    strncpy(rname, rname_original, rname_len);
    reg_size rsize = reg.rsize;
    if (rsize == 0) {
        rsize = 4;
    }
    if (rsize == 1) {
        if (rname[0] == 'r') {
            rname[rname_len++] = 'b';
        } else if (rname[1] == 'i') {
            rname[rname_len++] = 'l';
        } else {
            rname[rname_len - 1] = 'l';
        }
    } else if (rsize == 2) {
        if (rname[0] == 'r') {
            rname[rname_len++] = 'w';
        }
    } else if (rsize == 4) {
        if (rname[0] == 'r') {
            rname[rname_len++] = 'd';
        } else {
            *--rname = 'e';
        }
    } else if (rsize == 8) {
        if (rname[0] != 'r') {
            *--rname = 'r';
        }
    } else {
        report_error("incorrect rsize %d\n", rsize);
    }
    buf_snprintf(buffer, "%s", rname);
}

void emit_mov(reg_t dst, i64 value) {
    if (value == 0) {
        emit_rr(STR("xor"), dst, dst);
    } else {
        emit_rx(STR("mov"), dst);
        buf_puts(fn_buf, STR(", "));
        buf_snprintf(fn_buf, "%"PRId64, value);
        buf_putc(fn_buf, '\n');
    }
}

void emit_mov_reg(reg_t dst, reg_t src) {
    const char *op = "mov";
    if (dst.rsize && src.rsize && dst.rsize > src.rsize) {
        if (src.dtype.base && src.dtype.base->sign) {
            if (dst.rsize == 8 && src.rsize == 4) {
                op = "movsxd";
            } else {
                op = "movsx";
            }
        } else {
            if (dst.rsize == 8 && src.rsize == 4) {
                dst.rsize = 4;
            } else {
                op = "movzx";
            }
        }
    } else if (dst.rsize < src.rsize) {
        dst.rsize = src.rsize;
    }
    emit_rr(STR(op), dst, src);
}

void type_conv(reg_t dst, reg_t src) {
    if (src.dtype.base) {
        src.rsize = (reg_size)src.dtype.base->size;
    }
    emit_mov_reg(dst, src);
}

void emit_lea_begin(reg_t dst, reg_t lhs, str op) {
    emit_rx(STR("lea"), dst);
    buf_puts(fn_buf, STR(", ["));
    lhs.rsize = 8; /* address operands must be 64-bit in long mode */
    buf_putreg(fn_buf, lhs);
    buf_puts(fn_buf, op);
}
void emit_lea_end(void) {
    buf_puts(fn_buf, STR("]\n"));
}

void emit_add(reg_t dst, reg_t lhs, i64 rhs) {
    emit_lea_begin(dst, lhs, STR("+"));
    buf_puti(fn_buf, rhs);
    emit_lea_end();
}

void emit_add_reg(reg_t dst, reg_t lhs, reg_t rhs) {
    emit_lea_begin(dst, lhs, STR("+"));
    rhs.rsize = 8; /* index must be 64-bit */
    buf_putreg(fn_buf, rhs);
    emit_lea_end();
}

void emit_sub(reg_t dst, reg_t lhs, i64 rhs) {
    emit_lea_begin(dst, lhs, STR("-"));
    buf_puti(fn_buf, rhs);
    emit_lea_end();
}

void emit_sub_reg(reg_t dst, reg_t lhs, reg_t rhs) {
    emit_rr(STR("mov"), dst, lhs);
    emit_rr(STR("sub"), dst, rhs);
}

void emit_cmp(reg_t lhs, i64 rhs) {
    emit_ri(STR("cmp"), lhs, rhs);
}

void emit_cmp_reg(reg_t lhs, reg_t rhs) {
    emit_rr(STR("cmp"), lhs, rhs);
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

    emit_rx(STR("lea"), dst);
    buf_snprintf(fn_buf, ", [rip+%s]\n", buffer);

    buf_snprintf(&cstr_buf, "%s:\n", buffer);
    buf_puts(&cstr_buf, STR("\t.asciz "));
    buf_puts(&cstr_buf, *s);
    buf_putc(&cstr_buf, '\n');

    free(buffer);
}

void emit_lsl(reg_t dst, reg_t lhs, i64 rhs) {
    if (rhs >= 4) {
        emit_rr(STR("mov"), dst, lhs);
        emit_ri(STR("shl"), dst, rhs);
    } else if (rhs == 0) {
        emit_mov_reg(dst, lhs);
    } else if (rhs == 1) {
        emit_add_reg(dst, lhs, lhs);
    } else if (rhs <= 3) {
        emit_lea_begin(dst, lhs, STR("*"));
        buf_puti(fn_buf, 1 << rhs);
        emit_lea_end();
    }
}

const char *ptr_names[] = {
    "byte", "word", "dword", "qword"
};
int rsize_log2(reg_size size) {
    if (size <= 1)
        return 0;
    else if (size <= 2)
        return 1;
    else if (size <= 4)
        return 2;
    else if (size <= 8)
        return 3;
    else
        unreachable;
}

void emit_str_reg(reg_t dst, reg_t src, int offset) {
    buf_puts(fn_buf, STR("\tmov "));
    int index = rsize_log2(src.rsize);
    buf_snprintf(fn_buf, "%s ptr [", ptr_names[index]);
    buf_putreg(fn_buf, dst);
    buf_snprintf(fn_buf, " + %d], ", offset);
    buf_putreg(fn_buf, src);
    buf_putc(fn_buf, '\n');
}

const reg_t rax = {.rsize = sizeof (void *), .reg_type = SCRATCH, .dtype = {.decl = {{.tag = DK_ADDR, .amount = 1}}, .decl_len = 1}};
const reg_t rbp = {.rsize = sizeof (void *), .reg_type = FRAME, .dtype = {.decl = {{.tag = DK_ADDR, .amount = 1}}, .decl_len = 1}};

static void emit_mem_imm(int index, reg_t dst, int offset, i64 value) {
    buf_puts(fn_buf, STR("\tmov "));
    buf_snprintf(fn_buf, "%s ptr [", ptr_names[index]);
    buf_putreg(fn_buf, dst);
    buf_snprintf(fn_buf, " + %d], %"PRId64"\n", offset, value);
}

void emit_str_imm(reg_t dst, i64 value, int offset) {
    int index = rsize_log2((reg_size)dst.dtype.base->size);
    if (index >= 3 && (value < INT32_MIN || value > INT32_MAX)) {
        /* a 64-bit immediate has no direct mem form; store it as two dwords */
        emit_mem_imm(2, dst, offset, (i64)(i32)(u32)value);
        emit_mem_imm(2, dst, offset + 4, (i64)(i32)(u32)((u64)value >> 32));
        return;
    }
    emit_mem_imm(index, dst, offset, value);
}

void emit_ldr(reg_t dst, reg_t src, int offset) {
    buf_puts(fn_buf, STR("\tmov "));
    buf_putreg(fn_buf, dst);

    int index = rsize_log2(dst.rsize);
    buf_snprintf(fn_buf, ", %s ptr [", ptr_names[index]);
    buf_putreg(fn_buf, src);
    buf_snprintf(fn_buf, " + %d]\n", offset);
}

void emit_str_regoff(reg_t dst, reg_t src, reg_t offset) {
    emit_lea_begin(rax, dst, STR("+"));
    offset.rsize = 8; /* index must be 64-bit */
    buf_putreg(fn_buf, offset);
    emit_lea_end();

    emit_str_reg(rax, src, 0);
}

void emit_str_imm_regoff(reg_t dst, i64 value, reg_t offset) {
    emit_lea_begin(rax, dst, STR("+"));
    offset.rsize = 8; /* index must be 64-bit */
    buf_putreg(fn_buf, offset);
    emit_lea_end();

    reg_t addr = rax;
    addr.dtype.base = dst.dtype.base;
    emit_str_imm(addr, value, 0);
}

void emit_ldr_reg(reg_t dst, reg_t src, reg_t offset) {
    emit_lea_begin(dst, src, STR("+"));
    offset.rsize = 8; /* index must be 64-bit */
    buf_putreg(fn_buf, offset);
    emit_lea_end();

    emit_ldr(dst, dst, 0);
}

/* Pack up to 8 bytes of struct fields into dst using x86_64 instructions.
   Returns true if dst was written to, false if it remains unset. */
bool emit_eightbyte_struct(reg_t dst, const dtype_t *dtype, const dyn_agg_member *args,
                           int *index, size_t *size_out, size_t limit) {
    (void)limit;
    type_t *type = dtype->base;
    ptrdiff_t member_count = args->cur - args->begin;

    bool is_arr = !dtype_empty(dtype) && dtype_top(dtype).tag == DK_ARRAY;

    /* For arrays, size dst to the bytes packed into this eightbyte so the
       packing uses a wide-enough register (e.g. rax, not eax). */
    if (is_arr) {
        size_t remaining = (size_t)(member_count - *index) * dtype->base->size;
        size_t span = remaining > 8 ? 8 : remaining;
        dst.rsize = span > 4 ? 8 : (span > 2 ? 4 : (span > 1 ? 2 : 1));
    }

    /* Shift scratch — must differ from dst. When packing the hi eightbyte
       dst is SCRATCH[1], so bump past it (and past the lo eightbyte SCRATCH[0],
       which must survive until the store). */
    reg_t shift_tmp = {.reg_type = SCRATCH, .offset = 1, .rsize = 8};
    if (dst.reg_type == SCRATCH && dst.offset >= shift_tmp.offset) {
        shift_tmp.offset = dst.offset + 1;
    }

    bool cleared = false;
    size_t size_acc = 0;

    for (ptrdiff_t i = *index; i < member_count; ++i, *index = (int)i) {
        member_t local_memb;
        member_t *memb;
        if (is_arr) {
            local_memb = (member_t){.dtype = *dtype, .offset = (size_t)i * dtype->base->size};
            memb = &local_memb;
        } else {
            memb = &type->struct_t.members.begin[i];
        }
        size_t memb_size = is_arr ? dtype->base->size : dtype_size(&memb->dtype);
        size_t offset_bits = memb->offset * 8;
        size_t local_offset_bits = offset_bits % 64;

        size_acc += memb_size;
        if (size_acc > 8) {
            size_acc -= memb_size;
            *index = (int)i;
            break;
        }

        agg_member *arg = &args->begin[i];
        if (arg->tag == VALUE) {
            i64 shifted = (i64)((u64)arg->value << local_offset_bits);
            if (!cleared) {
                emit_mov(dst, shifted);
                cleared = true;
            } else if (shifted != 0) {
                /* x86_64 or-immediate is 32-bit sign-extended; use scratch for wider values */
                if (shifted == (i32)shifted) {
                    emit_ri(STR("or"), dst, shifted);
                } else {
                    emit_mov(shift_tmp, shifted);
                    emit_rr(STR("or"), dst, shift_tmp);
                }
            }
        } else if (arg->tag == REG) {
            reg_t reg = arg->reg;
            if (reg.reg_type == STACK) {
                /* STACK reg is a variable at rbp-offset; compute its address first */
                const reg_t frame = {.reg_type = FRAME, .rsize = 8};
                emit_sub(shift_tmp, frame, (i64)reg.offset);
                reg = shift_tmp;
            }
            reg.rsize = memb_size < 4 ? (reg_size)memb_size : (memb_size <= 4 ? 4 : 8);

            if (local_offset_bits == 0 && reg.rsize == dst.rsize) {
                /* Sizes match: direct move or OR */
                if (!cleared) {
                    emit_rr(STR("mov"), dst, reg);
                } else {
                    emit_rr(STR("or"), dst, reg);
                }
            } else {
                /* Zero-extend src to shift_tmp (rcx, 64-bit) first */
                if (reg.rsize < 8) {
                    emit_mov_reg(shift_tmp, reg);
                } else {
                    emit_rr(STR("mov"), shift_tmp, reg);
                }
                if (local_offset_bits != 0) {
                    emit_ri(STR("shl"), shift_tmp, (i64)local_offset_bits);
                }
                if (!cleared) {
                    emit_rr(STR("mov"), dst, shift_tmp);
                } else {
                    emit_rr(STR("or"), dst, shift_tmp);
                }
            }
            cleared = true;
        }
    }

    *size_out += size_acc;
    return cleared;
}

void emit_zero_out(reg_t dst) {
    emit_rr(STR("xor"), dst, dst);
}

void emit_zerofill(reg_t base, i64 offset, const dtype_t *dtype) {
    size_t size = dtype_size(dtype);
    reg_t zero = {.reg_type = SCRATCH, .offset = 1, .rsize = 8, .dtype = {.base = dtype->base}};
    emit_rr(STR("xor"), zero, zero);

    reg_size rsize = 8;
    while (size > 0) {
        while (size >= rsize) {
            zero.rsize = rsize;
            emit_str_reg(base, zero, (int)offset);
            size -= rsize;
            offset += rsize;
        }
        if (rsize == 1) break;
        rsize /= 2;
    }
}

void emit_store_eightbytes(reg_t base, i64 offset, reg_t lo, bool lo_written,
                           reg_t hi, bool hi_written, bool has_hi) {
    if (has_hi) {
        lo.rsize = 8;
        hi.rsize = 8;
    }
    if (!lo_written) {
        emit_rr(STR("xor"), lo, lo);
    }
    emit_str_reg(base, lo, (int)offset);
    if (has_hi) {
        if (!hi_written) {
            emit_rr(STR("xor"), hi, hi);
        }
        emit_str_reg(base, hi, (int)offset + 8);
    }
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
            emit_ri(STR("shr"), full, (i64)chunk * 8);
        }
    }
}

void emit_make_array(reg_t dst, type_t *type, u32 len, dyn_regable *args) {
    (void)dst; (void)type; (void)len; (void)args;
}

void emit_store_array(reg_t dst, i64 offset, type_t *type, u32 len, dyn_regable *args) {
    (void)dst; (void)offset; (void)type; (void)len; (void)args;
}

void emit_array_access(reg_t dst, reg_t src, reg_t offset, load_store_t is_store) {
    dtype_t *dtype = &src.dtype;
    size_t elem_size = dtype->base->size;

    if (src.reg_type == STACK) {
        if (src.offset) {
            reg_t tmp_src = src;
            tmp_src.reg_type = SCRATCH;
            tmp_src.offset = 2;
            tmp_src.rsize = 8;
            const reg_t frame = {.reg_type = FRAME, .rsize = 8};
            emit_sub(tmp_src, frame, src.offset);
            src = tmp_src;
        }
    }

    if (elem_size == 0) {
        report_error("element size was zero\n");
        return;
    }

    if (offset.rsize == 1 || offset.rsize == 2) {
        reg_t wide = offset;
        wide.rsize = 8;
        emit_mov_reg(wide, offset);
        offset = wide;
    }
    offset.rsize = 8;
    src.rsize = 8;

    if (elem_size == 1) {
        if (is_store)
            emit_str_regoff(src, dst, offset);
        else
            emit_ldr_reg(dst, src, offset);
        return;
    }

    if (elem_size <= 8) {
        int exp = power_of_two_exponent(elem_size);
        if (exp) {
            int log2 = rsize_log2((reg_size)dst.rsize);
            if (is_store) {
                buf_snprintf(fn_buf, "\tmov %s ptr [", ptr_names[log2]);
                buf_putreg(fn_buf, src);
                buf_puts(fn_buf, STR(" + "));
                buf_putreg(fn_buf, offset);
                buf_snprintf(fn_buf, " * %zu], ", elem_size);
                buf_putreg(fn_buf, dst);
                buf_putc(fn_buf, '\n');
            } else {
                buf_puts(fn_buf, STR("\tmov "));
                buf_putreg(fn_buf, dst);
                buf_snprintf(fn_buf, ", %s ptr [", ptr_names[log2]);
                buf_putreg(fn_buf, src);
                buf_puts(fn_buf, STR(" + "));
                buf_putreg(fn_buf, offset);
                buf_snprintf(fn_buf, " * %zu]\n", elem_size);
            }
            return;
        }
    }

    /* General case: compute byte offset = index * elem_size, then access */
    reg_t idx = {.reg_type = SCRATCH, .offset = 1, .rsize = 8};
    emit_mov_reg(idx, offset);
    buf_snprintf(fn_buf, "\timul ");
    buf_putreg(fn_buf, idx);
    buf_snprintf(fn_buf, ", %zu\n", elem_size);
    emit_add_reg(idx, src, idx);

    if (is_store)
        emit_str_reg(idx, dst, 0);
    else
        emit_ldr(dst, idx, 0);
}

void emit_elem_addr(reg_t dst, reg_t object, reg_t index) {
    reg_t base = {.reg_type = SCRATCH, .offset = 0, .rsize = sizeof (void *)};
    if (object.reg_type == STACK) {
        const reg_t frame = {.reg_type = FRAME, .rsize = sizeof (void *)};
        emit_sub(base, frame, object.offset);
    } else {
        emit_mov_reg(base, object);
    }

    const size_t elem_size = object.dtype.base->size;
    if (index.rsize == 1 || index.rsize == 2) {
        reg_t wide = index;
        wide.rsize = 8;
        emit_mov_reg(wide, index);
        index = wide;
    }
    index.rsize = 8;
    if (elem_size <= 8 && (elem_size == 1 || power_of_two_exponent(elem_size))) {
        emit_lea_begin(dst, base, STR("+"));
        buf_putreg(fn_buf, index);
        buf_snprintf(fn_buf, "*%zu]\n", elem_size);
    } else {
        reg_t idx = {.reg_type = SCRATCH, .offset = 1, .rsize = 8};
        emit_mov_reg(idx, index);
        buf_snprintf(fn_buf, "\timul ");
        buf_putreg(fn_buf, idx);
        buf_snprintf(fn_buf, ", %zu\n", elem_size);
        emit_add_reg(dst, base, idx);
    }
}

/* --- control flow --- */

void emit_branch(str fn_name, str label, int index) {
    buf_puts(fn_buf, STR("\tjmp "));
    put_label(fn_name, label, index);
    buf_puts(fn_buf, STR("\n"));
}

const char *const cond_str[] = {
    "e", "ne", "ge", "l", "g", "le", "ae", "a",
};
bool emit_branch_cond(cond_t condition, str fn_name, str label, int index) {
    if (condition >= (cond_t)(sizeof cond_str / sizeof cond_str[0])) {
        fprintf(stderr, "unknown condition %d", condition);
        return false;
    }
    buf_snprintf(fn_buf, "\tj%s ", cond_str[condition]);
    put_label(fn_name, label, index);
    buf_puts(fn_buf, STR("\n"));
    return true;
}

void emit_label(str fn_name, str label, int index) {
    put_label(fn_name, label, index);
    buf_puts(fn_buf, STR(":\n"));
}

void emit_cond_set(reg_t dst, cond_t cond) {
    if (cond >= (cond_t)(sizeof cond_str / sizeof cond_str[0])) {
        fprintf(stderr, "unknown condition %d", cond);
        return;
    }
    reg_t byte_dst = dst;
    byte_dst.rsize = 1;
    buf_snprintf(fn_buf, "\tset%s ", cond_str[cond]);
    buf_putreg(fn_buf, byte_dst);
    buf_putc(fn_buf, '\n');

    reg_t wide_dst = dst;
    if (wide_dst.rsize <= 1)
        wide_dst.rsize = 4;
    buf_puts(fn_buf, STR("\tmovzx "));
    buf_putreg(fn_buf, wide_dst);
    buf_puts(fn_buf, STR(", "));
    buf_putreg(fn_buf, byte_dst);
    buf_putc(fn_buf, '\n');
}

/* --- function management --- */

void emit_fn_prologue_epilogue(const parser_context *parser_context) {
    size_t stack_size = 0;
    size_t shadow_size = 0;
    bool calls_fn = parser_context->calls_fn;
    if (calls_fn) {
        shadow_size = 32;
        stack_size += shadow_size; // for shadow space (x64 abi)
    }
    size_t locals_size = (size_t)parser_context->stack_size;
    stack_size += locals_size;
    stack_size = ALIGN_TO(stack_size, (size_t)0x10);

    if (locals_size)
        buf_puts(&context->prologue_buf, STR_FROM_INSTR("push rbp"));
    int regs_to_save = parser_context->nreg_count;
    int tmp = regs_to_save + (locals_size ? 1 : 0) + (calls_fn ? 1 : 0);
    if (tmp % 2 == 1)
        stack_size += 8; // for aligning stack to 0x10 bytes on 'call' (x64 abi)

    for (int i = 0; i < regs_to_save; ++i) {
        emit_r(&context->prologue_buf, "push", (reg_t){.reg_type = NREG, .offset = i, .rsize = sizeof (void *)});
    }
    if (locals_size) {
        if (shadow_size)
            buf_snprintf(&context->prologue_buf, "\tlea rbp, [rsp - 0x%zx]\n", shadow_size);
        else
            buf_snprintf(&context->prologue_buf, "\tmov rbp, rsp\n");
    }
    if (stack_size)
        buf_snprintf(&context->prologue_buf, "\tsub rsp, 0x%zx\n", stack_size);


    if (stack_size)
        buf_snprintf(fn_buf, "\tadd rsp, 0x%zx\n", stack_size);
    for (int i = regs_to_save - 1; i >= 0; --i) {
        emit_r(fn_buf, "pop", (reg_t){.reg_type = NREG, .offset = i, .rsize = sizeof (void *)});
    }
    if (locals_size)
        buf_puts(fn_buf, STR_FROM_INSTR("pop rbp"));
}

void emit_fn_call(const str *s) {
    buf_puts(fn_buf, STR("\tcall "));
    buf_puts(fn_buf, *s);
    buf_putc(fn_buf, '\n');
}

void emit_fn(str fn_name) {
    buf_puts(&context->fn_header_buf, STR("\n\t.globl "));
    buf_puts(&context->fn_header_buf, STR(fn_prefix));
    buf_puts(&context->fn_header_buf, fn_name);
    buf_puts(&context->fn_header_buf, STR("\n\t.p2align 4\n"));
    if (*fn_annotation_fmt) {
        buf_snprintf(&context->fn_header_buf, fn_annotation_fmt,
                     (int)str_len(fn_name), fn_name.data);
    }
    buf_puts(&context->fn_header_buf, STR(fn_prefix));
    buf_puts(&context->fn_header_buf, fn_name);
    buf_puts(&context->fn_header_buf, STR(":\n"));
}

void emit_ret(void) {
    buf_puts(fn_buf, STR_FROM_INSTR("ret"));
}
