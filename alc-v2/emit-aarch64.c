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

DECL_PTR(static buf, text_buf);
DECL_PTR(static buf, cstr_buf);

DECL_PTR(static buf, fn_header_buf);
DECL_PTR(static buf, prologue_buf);
DECL_PTR(static buf, fn_buf);

char *cstr_begin = NULL;
unsigned string_lit_counts = 0;

extern const char *addrgen_adrp;
extern const char *addrgen_add;
extern const char *fn_prefix;
extern const char *mainfn_annotation;
extern const char *local_string_prefix;
extern type_t *type_comptime_int;


const char *const cond_str[] = {
    "eq",
};

void emit_init(void) {
    buf_init(text_buf, INIT_BUFSIZ);
    buf_puts(text_buf, STR_FROM(text_section_header));

    buf_init(cstr_buf, INIT_BUFSIZ);
    buf_puts(cstr_buf, STR_FROM(string_section_header));
    cstr_begin = cstr_buf->cur;

    emit_reset_fn();
}

void emit_reset_fn(void) {
    buf_init(fn_header_buf, 0x100);
    buf_init(prologue_buf, INIT_BUFSIZ);
    buf_init(fn_buf, INIT_BUFSIZ);
}

void emit_fnbuf(FILE *out) {
    buf_fwrite(fn_header_buf, out);
    buf_fwrite(prologue_buf, out);
    buf_fwrite(fn_buf, out);
}

void emit_text(FILE *out) {
    buf_fwrite(text_buf, out);
}

void emit_cstr(FILE *out) {
    if (cstr_begin < cstr_buf->cur) {
        buf_fwrite(cstr_buf, out);
    }
}

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
        compile_err(NULL, CSI_RED"aarch64: cannot load size bigger than 8 to register (was %d)\n"CSI_RESET, reg_size);
        format = "x";
    }
    return format;
}

static void buf_putreg(buf *buffer, reg_t reg) {
    if (reg.reg_type == STACK) {
        buf_puts(buffer, STR_FROM("sp"));
    } else {
        const char *format;
        if (reg.rsize <= 4) {
            format = "w%d";
        } else if (reg.rsize <= 8) {
            format = "x%d";
        } else {
            compile_err(NULL, CSI_RED"aarch64: cannot load size bigger than 8 to register (was %d)\n"CSI_RESET, reg.rsize);
			return;
        }
        buf_snprintf(buffer, format, get_regoff(reg));
    }
}

void eightbyte_make_struct(reg_t dst, type_t *type, dyn_regable *args, int *index, size_t *size) {
    const dyn_member_t *members = &type->struct_t.members;
    ptrdiff_t member_count = members->cur - members->begin;
    bool cleared = false;

    if (type->size > 8) {
        dst.rsize = 8;
    } else {
        dst.rsize = (reg_size)type->size;
    }

    size_t size_acc = 0;
    size_t base_offset = 0;
    for (ptrdiff_t i = *index; i < member_count; ++i, *index = (int)i) {
        regable *r = &args->begin[i];
        member_t *memb = &members->begin[i];
        type_t *memb_type = memb->type;
        size_t memb_size = memb_type->size;

        size_t offset = memb->offset * 8;
        if (!size_acc) {
            base_offset = offset;
        }
        offset -= base_offset;
        size_acc += memb_size;
        if (size_acc > 8) {
            size_acc -= memb_size;
            *index = (int)i;
            break;
        }

        if (r->tag == VALUE) {
            i64 value = r->value;
            if (offset % 16 == 0) {
                size_t size = memb->type->size;
                while (size < 2) {
                    if (++i >= member_count)
                        break;
                    regable *r = args->begin + i;
                    if (r->tag != VALUE) {
                        --i;
                        break;
                    }
                    member_t *memb = &members->begin[i];
                    if (size + memb->type->size > 2) {
                        --i;
                        break;
                    }
                    size += memb->type->size;
                    value |= r->value << memb->type->size * 8;
                }
            }

            if (value == 0)
                continue;

            if (offset % 16 != 0) {
                if (!cleared) {
                    emit_ri(STR_FROM("mov"), dst, value << offset);
                    unreachable;
                }
                emit_rri(STR_FROM("orr"), dst, dst, value << offset);
                continue;
            } else {
                if (!cleared) {
                    emit_risi(STR("movz"), dst, value, STR("lsl"), (i64)offset);
                } else {
                    emit_risi(STR("movk"), dst, value, STR("lsl"), (i64)offset);
                }
            }
            cleared = true;
        } else if (r->tag == REG) {
            reg_t reg = r->reg;

            if (offset == 0) {
                if (memb_size < 4) {
                    int mask = 0xff;
                    for (size_t i = 1; i < memb_size; ++i) {
                        mask |= 0xff << i * 8;
                    }
                    if (reg.rsize < dst.rsize)
                        reg.rsize = dst.rsize;
                    emit_rri(STR_FROM("and"), dst, reg, mask);
                } else {
                    reg_t tmp_dst = dst;
                    if (reg.rsize < tmp_dst.rsize) // no need to emit mov?
                        tmp_dst.rsize = reg.rsize;
                    emit_rr(STR_FROM("mov"), tmp_dst, reg);
                }
            } else {
                reg_t reg = r->reg;
                if (reg.rsize < dst.rsize)
                    reg.rsize = dst.rsize;
                i64 width = (i64)memb_size * 8;
                if (cleared)
                    emit_rrii(STR_FROM("bfi"), dst, reg, (i64)offset, width);
                else
                    emit_rrii(STR("ubfiz"), dst, reg, (i64)offset, width);
            }
            cleared = true;
        } else {
            unreachable;
        }
    }

    if (!cleared) {
        emit_mov(dst, 0);
    }
    *size += size_acc;
}

void emit_make_array(reg_t dst, type_t *type, u32 len, dyn_regable *args) {
    ptrdiff_t member_count = len;
    bool cleared = false;

    if (type->size > 8) {
        dst.rsize = 8;
    }

    int base_regoff = dst.offset;
    size_t size_acc = 0;
    for (ptrdiff_t i = 0; i < member_count; ++i) {
        regable *r = &args->begin[i];
        type_t *memb_type = type;
        size_t memb_size = memb_type->size;
        size_acc += memb_size;

        size_t offset = memb_size * (size_t)i  * 8;
        if (size_acc > 8) {
            int new_offset = base_regoff + (int)(size_acc - 1)/ 8;
            if (dst.offset != new_offset) {
                cleared = false;
                dst.offset = new_offset;
            }
            offset -= 8 * 8;
        }

        if (r->tag == VALUE) {
            i64 value = r->value;
            if (offset % 16 == 0) {
                size_t size = type->size;
                while (size < 2) {
                    if (++i >= member_count)
                        break;
                    regable *r = args->begin + i;
                    if (r->tag != VALUE) {
                        --i;
                        break;
                    }
                    if (size + type->size > 2) {
                        --i;
                        break;
                    }
                    size += type->size;
                    value |= r->value << type->size * 8;
                }
            }

            if (value == 0)
                continue;

            if (offset % 16 != 0) {
                if (!cleared) {
                    emit_ri(STR_FROM("mov"), dst, value << offset);
                    unreachable;
                }
                emit_rri(STR_FROM("orr"), dst, dst, value << offset);
                continue;
            } else {
                if (!cleared) {
                    emit_risi(STR("movz"), dst, value, STR("lsl"), (i64)offset);
                } else {
                    emit_risi(STR("movk"), dst, value, STR("lsl"), (i64)offset);
                }
            }
            cleared = true;
        } else if (r->tag == REG) {
            reg_t reg = r->reg;

            if (offset == 0) {
                if (memb_size < 4) {
                    int mask = 0xff;
                    for (size_t i = 1; i < memb_size; ++i) {
                        mask |= 0xff << i * 8;
                    }
                    if (reg.rsize < dst.rsize)
                        reg.rsize = dst.rsize;
                    emit_rri(STR_FROM("and"), dst, reg, mask);
                } else {
                    reg_t tmp_dst = dst;
                    if (reg.rsize < tmp_dst.rsize) // no need to emit mov?
                        tmp_dst.rsize = reg.rsize;
                    emit_rr(STR_FROM("mov"), tmp_dst, reg);
                }
            } else {
                reg_t reg = r->reg;
                if (reg.rsize < dst.rsize)
                    reg.rsize = dst.rsize;
                i64 width = (i64)memb_size * 8;
                if (cleared)
                    emit_rrii(STR_FROM("bfi"), dst, reg, (i64)offset, width);
                else
                    emit_rrii(STR("ubfiz"), dst, reg, (i64)offset, width);
            }
            cleared = true;
        } else {
            unreachable;
        }
    }

    if (!cleared) {
        emit_mov(dst, 0);
    }
}

void emit_make_struct(reg_t dst, type_t *type, dyn_regable *args) {
    const dyn_member_t *members = &type->struct_t.members;
    ptrdiff_t member_count = members->cur - members->begin;
    bool cleared = false;

    if (type->size > 8) {
        dst.rsize = 8;
    }

    int base_regoff = dst.offset;
    size_t size_acc = 0;
    for (ptrdiff_t i = 0; i < member_count; ++i) {
        regable *r = &args->begin[i];
        member_t *memb = &members->begin[i];
        type_t *memb_type = memb->type;
        size_t memb_size = memb_type->size;
        size_acc += memb_size;

        size_t offset = memb->offset * 8;
        if (size_acc > 8) {
            int new_offset = base_regoff + (int)(size_acc - 1)/ 8;
            if (dst.offset != new_offset) {
                cleared = false;
                dst.offset = new_offset;
            }
            offset -= 8 * 8;
        }

        if (r->tag == VALUE) {
            i64 value = r->value;
            if (offset % 16 == 0) {
                size_t size = memb->type->size;
                while (size < 2) {
                    if (++i >= member_count)
                        break;
                    regable *r = args->begin + i;
                    if (r->tag != VALUE) {
                        --i;
                        break;
                    }
                    member_t *memb = &members->begin[i];
                    if (size + memb->type->size > 2) {
                        --i;
                        break;
                    }
                    size += memb->type->size;
                    value |= r->value << memb->type->size * 8;
                }
            }

            if (value == 0)
                continue;

            if (offset % 16 != 0) {
                if (!cleared) {
                    emit_ri(STR_FROM("mov"), dst, value << offset);
                    unreachable;
                }
                emit_rri(STR_FROM("orr"), dst, dst, value << offset);
                continue;
            } else {
                if (!cleared) {
                    emit_risi(STR("movz"), dst, value, STR("lsl"), (i64)offset);
                } else {
                    emit_risi(STR("movk"), dst, value, STR("lsl"), (i64)offset);
                }
            }
            cleared = true;
        } else if (r->tag == REG) {
            reg_t reg = r->reg;

            if (offset == 0) {
                if (memb_size < 4) {
                    int mask = 0xff;
                    for (size_t i = 1; i < memb_size; ++i) {
                        mask |= 0xff << i * 8;
                    }
                    if (reg.rsize < dst.rsize)
                        reg.rsize = dst.rsize;
                    emit_rri(STR_FROM("and"), dst, reg, mask);
                } else {
                    reg_t tmp_dst = dst;
                    if (reg.rsize < tmp_dst.rsize) // no need to emit mov?
                        tmp_dst.rsize = reg.rsize;
                    emit_rr(STR_FROM("mov"), tmp_dst, reg);
                }
            } else {
                reg_t reg = r->reg;
                if (reg.rsize < dst.rsize)
                    reg.rsize = dst.rsize;
                i64 width = (i64)memb_size * 8;
                if (cleared)
                    emit_rrii(STR_FROM("bfi"), dst, reg, (i64)offset, width);
                else
                    emit_rrii(STR("ubfiz"), dst, reg, (i64)offset, width);
            }
            cleared = true;
        } else {
            unreachable;
        }
    }

    if (!cleared) {
        emit_mov(dst, 0);
    }
}

static void emit_stp(reg_t src1, reg_t src2, reg_t base, i64 offset) {
    emit_rrx(STR("stp"), src1, src2);
    buf_puts(fn_buf, STR(", ["));
    buf_putreg(fn_buf, base);
    buf_snprintf(fn_buf, ", #%"PRId64"]\n", offset);
}

void emit_store_struct(reg_t dst, i64 offset, type_t *type, dyn_regable *args) {
    reg_size rsize = type->size > 8 ? 8 : (reg_size)type->size;
    reg_t tmp = {.reg_type = SCRATCH, .type = type, .rsize = rsize};

    const dyn_member_t *members = &type->struct_t.members;
    ptrdiff_t member_count = members->cur - members->begin;
    pi(member_count)
    int index = 0;
    size_t size = 0;

    while (index < member_count) {
        size_t member_off = size;
        eightbyte_make_struct(tmp, type, args, &index, &size);

        size_t remaining_size = type->size - size;
        if (remaining_size >= 8) {
            reg_t tmp2 = tmp;
            tmp2.offset++;
            eightbyte_make_struct(tmp2, type, args, &index, &size);
            remaining_size = type->size - size;
            emit_stp(tmp, tmp2, dst, offset + (i64)member_off);
        } else {
            emit_str(tmp, dst, (int)offset + (int)member_off);
        }
    }
}


void emit_store_array(reg_t dst, i64 offset, type_t *type, u32 len, dyn_regable *args) {
    ptrdiff_t member_count = len;
    int index = 0;
    size_t size = type->size;

    if (type->tag == TK_STRUCT) {
        while (index < member_count) {
            pi(index)
            pi(offset)
            emit_store_struct(dst, offset, type, args);

            ++index;
            offset += size;
        }
    } else if (type->tag == TK_FUND) {
        reg_size rsize = type->size > 8 ? 8 : (reg_size)type->size;
        reg_t tmp = {.reg_type = SCRATCH, .type = type, .rsize = rsize};
        while (index < member_count) {
            size_t member_off = (size_t)index *size;
            emit_str(tmp, dst, (int)offset + (int)member_off);
            ++index;
        }
    } else unreachable;
}
void emit_mov(reg_t dst, i64 value) {
    int regidx = get_regoff(dst);
    if (dst.type == type_comptime_int) {
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
        if (!dst.type || !dst.type->sign)
            buf_snprintf(fn_buf, INSTR("mov %s%d, #%"PRIx64), get_wx(dst.rsize), regidx, value);
        else
            buf_snprintf(fn_buf, INSTR("mov %s%d, #%"PRIi64), get_wx(dst.rsize), regidx, value);
    }

}

void type_conv(reg_t dst, reg_t src) {
    type_t *srct = src.type;
    if (!srct) // TODO is it okay to ignore?
        return;

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
    if (dst.addr) {
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

    buf_snprintf(cstr_buf, "%s:\n", buffer);
    buf_puts(cstr_buf, STR_FROM("\t.asciz "));
    buf_puts(cstr_buf, *s);
    buf_putc(cstr_buf, '\n');

    free(buffer);
}

void emit_lsl(reg_t dst, reg_t lhs, i64 rhs) {
    emit_rri(STR_FROM("lsl"), dst, lhs, rhs);
}

static void load_store_x(const char *op, reg_t r0, reg_t r1) {
    const char *suffix = "";
    size_t size = r0.type->size;
    if (r0.addr) {
        size = 8;
    }
    if (r0.rsize <= 0) {
        compile_err(NULL, "cannot %s size of zero\n", op);

        printd("dump r0 | size: %d, reg_type: %d, offset: %d\n", r0.rsize, r0.reg_type, r0.offset);
        report_error("");
    } else if (size <= 1) {
        if (*op == 'l' && r0.type->sign)
            suffix = "sb";
        else
            suffix = "b";
    } else if (size <= 2) {
        if (*op == 'l' && r0.type->sign)
            suffix = "sh";
        else
            suffix = "h";
    } else if (*op == 'l' && size == 8 && size == 4) {
        if (*op == 'l' && r0.type->sign)
            suffix = "sw";
    }
    buf_snprintf(fn_buf, ("\t%s%s %s%d, [x%d, "),
            op, suffix, get_wx(r0.rsize), get_regoff(r0), get_regoff(r1));
}

void emit_str(reg_t src, reg_t dst, int offset) {
    load_store_x("str", src, dst);
    buf_snprintf(fn_buf, ("#%d]\n"), offset);
}

void emit_ldr(reg_t dst, reg_t src, int offset) {
    load_store_x("ldr", dst, src);
    buf_snprintf(fn_buf, ("#%d]\n"), offset);
}

void emit_str_reg(reg_t src, reg_t dst, reg_t offset) {
    load_store_x("str", src, dst);
    buf_snprintf(fn_buf, ("x%d]\n"), get_regoff(offset));
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

bool emit_branch_cond(cond condition, str fn_name, str label, int index) {
    buf_puts(fn_buf, STR_FROM("\tb."));
    if (condition >= (cond)(sizeof (cond_str) / sizeof cond_str[0])) {
        fprintf(stderr, "unknown condition %d", condition);
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

void emit_fn_prologue_epilogue(const parser_context *context) {
    prologue_buf->cur = prologue_buf->start;
    if (!context->calls_fn
            && context->max_nreg_count == 0
            && context->stack_size == 0)
        return;

    int regs_to_save = context->max_nreg_count;
    if (regs_to_save + CALLEE_START >= 28) {
        compile_err(&context->cur_token, "used up all callee-saved registers");
        return;
    }
    bool calls_fn = context->calls_fn;
    if (calls_fn) {
        regs_to_save += 2;
    }

    int stack_size =
        ALIGN_TO(regs_to_save * (signed)sizeof (u64), 16)
        + ALIGN_TO(context->stack_size, 16);
    printd("\nstack report for: ");
    str_printd(&context->name);
    printd("\t- regs to save: %d, stack size: %d\n", regs_to_save, context->stack_size);
    printd("\t- aligned regs: %d, aligned stack: %d\n",
            ALIGN_TO(regs_to_save * (signed)sizeof (u64), 16),
            ALIGN_TO(context->stack_size, 16));
    printd("\t- result stack: %d\n", stack_size);

    int cur_stackoff = ALIGN_TO(context->stack_size, 16);
    const int stack_objs_size = cur_stackoff;
    if (stack_objs_size > 0) {
        buf_snprintf(prologue_buf, INSTR("sub sp, sp, #%d"), stack_size);
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
        buf_snprintf(prologue_buf, stp_format, reg0, reg1, off);
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
        buf_snprintf(prologue_buf, INSTR("stp x%d, x%d, [sp, #%d]"),
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
        buf_snprintf(prologue_buf, stp_format,
                reg0, off);
        buf_snprintf(fn_buf, ldp_format,
                reg0, off);
        cur_stackoff += 16;
    }

    if (stack_objs_size == 0) {
        buf_puts(prologue_buf, STR_FROM_INSTR("mov x29, sp"));
    } else {
        buf_snprintf(prologue_buf, INSTR("add x29, sp, #%d"), stack_objs_size);
    }

    if (defer_ldp) {
        const char *ldp_format = INSTR("ldp x%d, x%d, [sp], #%d");
        buf_snprintf(fn_buf, ldp_format, deferred0, deferred1, stack_size);
    }
    if (stack_objs_size > 0) {
        buf_snprintf(fn_buf, INSTR("add sp, sp, #%d"), stack_size);
    }
}

void emit_fn_call(const str *s) {
    buf_puts(fn_buf, STR_FROM("\tbl "));
    buf_puts(fn_buf, STR_FROM(fn_prefix));
    buf_puts(fn_buf, *s);
    buf_putc(fn_buf, '\n');
}

void emit_fn(str fn_name) {
    buf_puts(fn_header_buf, STR_FROM("\n\t.globl "));
    buf_puts(fn_header_buf, STR_FROM(fn_prefix));
	buf_puts(fn_header_buf, fn_name);
    buf_puts(fn_header_buf, STR_FROM("\n\t.p2align 2\n"));
    if (str_eq_lit(&fn_name, "main")) {
        buf_puts(fn_header_buf, STR_FROM(mainfn_annotation));
    }
    buf_puts(fn_header_buf, STR_FROM(fn_prefix));
	buf_puts(fn_header_buf, fn_name);
    buf_puts(fn_header_buf, STR_FROM(":\n"));
}

void emit_ret(void) {
    buf_puts(fn_buf, STR_FROM_INSTR("ret"));
}

#ifndef _WIN32
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 1, 2)))
#endif
void report_error(const char *format, ...) {
    int size = 0x100;
    void *array[size];
    size = backtrace(array, size);

    va_list args;
    va_start(args, format);
    fprintf(stderr, CSI_RED"error: "CSI_RESET);
    vfprintf(stderr, format, args);
    va_end(args);

    backtrace_symbols_fd(array, size, STDERR_FILENO);
}
#else
void report_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, CSI_RED"error: "CSI_RESET);
    vfprintf(stderr, format, args);
    va_end(args);
}
#endif
