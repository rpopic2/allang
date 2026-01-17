#include <stdio.h>
#include <inttypes.h>

#include "buffer.h"
#include "emit.h"
#include "str.h"
#include "err.h"

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
        printf(CSI_RED"cannot load size bigger than 8 to register (was %d)\n"CSI_RESET, reg_size);
        format = "x";
    }
    return format;
}

static void buf_putreg(buf *buffer, reg_t reg) {
    if (reg.reg_type == STACK) {
        buf_puts(buffer, STR_FROM("sp"));
    } else {
        const char *format;
        if (reg.size <= 4) {
            format = "w%d";
        } else if (reg.size <= 8) {
            format = "x%d";
        } else {
            printf("cannot load size bigger than 8 to regisetr");
        }
        buf_snprintf(buffer, format, get_regoff(reg));
    }
}

static void emit_rrx(str op, reg_t r0, reg_t r1) {
    buf_putc(fn_buf, '\t');
    buf_puts(fn_buf, op);
    buf_putc(fn_buf, ' ');
    buf_putreg(fn_buf, r0);
    buf_puts(fn_buf, STR_FROM(", "));
    buf_putreg(fn_buf, r1);
    buf_puts(fn_buf, STR_FROM(", "));
}

static void emit_rrr(str op, reg_t r0, reg_t r1, reg_t r2) {
    emit_rrx(op, r0, r1);
    buf_putreg(fn_buf, r2);
    buf_putc(fn_buf, '\n');
}

static void emit_rri(str op, reg_t r0, reg_t r1, i64 i0) {
    emit_rrx(op, r0, r1);
    buf_snprintf(fn_buf, "#%"PRId64, i0);
    buf_putc(fn_buf, '\n');
}

void emit_mov(reg_t dst, i64 value) {
    int regidx = get_regoff(dst);
    buf_snprintf(fn_buf, INSTR("mov w%d, #%"PRId64), regidx, value);
}

void emit_mov_reg(reg_t dst, reg_t src) {
    const char *format;
    if (dst.size <= 4) {
        format = INSTR("mov w%d, w%d");
    } else if (dst.size <= 8) {
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


static void load_store_x(const char *op, reg_t r0, reg_t r1) {
    const char *suffix = "";
    if (r0.size <= 0) {
        compile_err(NULL, "cannot %s size of zero\n", op);
        printd("dump r0 | size: %d, sign: %d, reg_type: %d, offset: %d\n", r0.size, r0.sign, r0.reg_type, r0.offset);
    } else if (r0.size <= 1) {
        suffix = "b";
    } else if (r0.size <= 2) {
        suffix = "h";
    }
    buf_snprintf(fn_buf, ("\t%s%s %s%d, [x%d, "),
            op, suffix, get_wx(r0.size), get_regoff(r0), get_regoff(r1));
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

static void put_label(str fn_name, str label, int index) {
    buf_putc(fn_buf, '.');
    buf_puts(fn_buf, fn_name);
    buf_putc(fn_buf, '.');
    buf_puts(fn_buf, label);
    if (index > 0) {
        buf_snprintf(fn_buf, "%d", index);
    }
}

void emit_branch(str fn_name, str label, int index) {
    buf_puts(fn_buf, STR_FROM("\tb "));
    put_label(fn_name, label, index);
    buf_puts(fn_buf, STR_FROM("\n"));
}

bool emit_branch_cond(cond condition, str fn_name, str label, int index) {
    buf_puts(fn_buf, STR_FROM("\tb."));
    if (condition >= (sizeof (cond_str) / sizeof cond_str[0])) {
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
            && context->nreg_count == 0
            && context->stack_size == 0)
        return;

    int regs_to_save = context->nreg_count;
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
    putc('\n', stdout);
    str_print(&context->name);
    printf("regs to save: %d, stack size: %d\n", regs_to_save, context->stack_size);
    printf("aligned regs: %d, aligned stack: %d\n",
            ALIGN_TO(regs_to_save * (signed)sizeof (u64), 16),
            ALIGN_TO(context->stack_size, 16));
    printf("result stack: %d\n", stack_size);

    int cur_stackoff = ALIGN_TO(context->stack_size, 16);
    const int stack_objs_size = cur_stackoff;
    if (stack_objs_size > 0) {
        buf_snprintf(prologue_buf, INSTR("sub sp, sp, #%d"), stack_size);
    }

    int remaining = regs_to_save;
    bool defer_ldp = false;

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
            stp_format = INSTR("stp x%d, x%d, [sp, #%d]!");
            defer_ldp = true;
            off = stack_size;
        }
        buf_snprintf(prologue_buf, stp_format, reg0, reg1, off);
        if (!defer_ldp)
            buf_snprintf(fn_buf, ldp_format, reg0, reg1, off);
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
        const char *ldp_format = INSTR("ldp x29, x30, [sp], #%d");
        buf_snprintf(fn_buf, ldp_format, stack_size);
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
