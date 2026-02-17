#include <inttypes.h>

#include "emit.h"
#include "buffer.h"
#include "err.h"
#include "emit_helper.h"
#include "typesys.h"

#define DECL_PTR(T, X) T _##X; T *X = &_##X
#define INSTR(s) "\t"s"\n"
#define STR_FROM_INSTR(s) STR_FROM(INSTR(s))

#define INIT_BUFSIZ 0x400

extern const char * const rname_scratch[];
extern const char * const rname_callee[];
extern const char * const rname_param[];
extern const char * const rname_ret[];
extern const size_t rname_scratch_len;
extern const size_t rname_callee_len;
extern const size_t rname_param_len;
extern const size_t rname_ret_len;

DECL_PTR(static buf, text_buf);
DECL_PTR(static buf, cstr_buf);

DECL_PTR(static buf, fn_header_buf);
DECL_PTR(static buf, prologue_buf);
DECL_PTR(static buf, fn_buf);

char *cstr_begin = NULL;
unsigned string_lit_counts = 0;

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

void emit_make_struct(reg_t dst, type_t *type, dyn_regable *args) {
	(void)dst, (void)type, (void)args;
}

static void buf_putreg(buf *buffer, reg_t reg) {
	register_dst reg_type = reg.reg_type;
    if (reg_type == STACK) {
        buf_puts(buffer, STR_FROM("rsp"));
		return;
    } else if (reg_type == FRAME) {
		buf_puts(buffer, STR_FROM("rbp"));
		return;
	}
	if (reg.offset < 0) {
		compile_err(NULL, "unexpected negative register offset %d", reg.offset);
        return;
	}
	size_t offset = (size_t)reg.offset;

	const char *rname_original;
#define RNAME_START if (false)
#define RNAME(RTYPE, rtype) \
	} else if (reg_type == RTYPE) { \
		if (offset >= rname_##rtype##_len) { \
			compile_err(NULL, "used up all "#rtype" registers. offset was %d\n", offset); \
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
		compile_err(NULL, "incorrect rsize %d\n", rsize);
	}
	printf("rname: %s\n", rname);
	buf_snprintf(buffer, "%s", rname);
}

void emit_mov(reg_t dst, i64 value) {
	printf("dst size: %d\n", dst.rsize);
	if (value == 0) {
		emit_rr(STR("xor"), dst, dst);
	} else {
		emit_ri(STR("mov"), dst, value);
	}
}

void emit_mov_reg(reg_t dst, reg_t src) {
    const char *op = "mov";
    if (dst.rsize && src.rsize && dst.rsize > src.rsize) {
        if (src.type->sign) {
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

void emit_lea_begin(reg_t dst, reg_t lhs, str op) {
	emit_rx(STR("lea"), dst);
	buf_puts(fn_buf, STR(", ["));
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
    int num_printed = snprintf(buffer, SPRINTF_BUFSIZ, ".Lstr.%d", string_lit_counts++);
    if (num_printed >= SPRINTF_BUFSIZ) {
        fputs("buffer overflow in snprintf\n", stderr);
        exit(EXIT_FAILURE);
    }

	emit_rx(STR("lea"), dst);
	buf_snprintf(fn_buf, ", [rip+%s]\n", buffer);

    buf_snprintf(cstr_buf, "%s:\n", buffer);
    buf_puts(cstr_buf, STR_FROM("\t.asciz "));
    buf_puts(cstr_buf, *s);
    buf_putc(cstr_buf, '\n');
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

void emit_str(reg_t src, reg_t dst, int offset) {
    buf_puts(fn_buf, STR("\tmov "));
    int index = rsize_log2(src.rsize);
    buf_snprintf(fn_buf, "%s ptr [", ptr_names[index]);
    buf_putreg(fn_buf, dst);
    buf_snprintf(fn_buf, " + %d], ", offset);
    buf_putreg(fn_buf, src);
    buf_putc(fn_buf, '\n');
}

void emit_ldr(reg_t dst, reg_t src, int offset) {
    buf_puts(fn_buf, STR("\tmov "));
    buf_putreg(fn_buf, dst);

    int index = rsize_log2(dst.rsize);
    buf_snprintf(fn_buf, ", %s ptr [", ptr_names[index]);
    buf_putreg(fn_buf, src);
    buf_snprintf(fn_buf, " + %d]\n", offset);
}

const reg_t rax = {.rsize = sizeof (void *), .reg_type = SCRATCH, .addr = 1};
const reg_t rbp = {.rsize = sizeof (void *), .reg_type = FRAME, .addr = 1};

void emit_str_reg(reg_t src, reg_t dst, reg_t offset) {
    emit_lea_begin(rax, dst, STR("+"));
    buf_putreg(fn_buf, offset);
    emit_lea_end();

    emit_str(src, rax, 0);
}

void emit_ldr_reg(reg_t dst, reg_t src, reg_t offset) {
    emit_lea_begin(dst, dst, STR("+"));
    buf_putreg(fn_buf, offset);
    emit_lea_end();

    emit_ldr(dst, rax, 0);
}


void emit_branch(str fn_name, str label, int index) {
    buf_puts(fn_buf, STR("\tjmp "));
    put_label(fn_name, label, index);
    buf_puts(fn_buf, STR_FROM("\n"));
}

const char *const cond_str[] = {
    "e",
};
bool emit_branch_cond(cond condition, str fn_name, str label, int index) {
    if (condition >= (cond)(sizeof cond_str / sizeof cond_str[0])) {
        fprintf(stderr, "unknown condition %d", condition);
        return false;
    }
    buf_snprintf(fn_buf, "\tj%s ", cond_str[condition]);
    put_label(fn_name, label, index);
    buf_puts(fn_buf, STR_FROM("\n"));
	return true;
}

void emit_label(str fn_name, str label, int index) {
    put_label(fn_name, label, index);
    buf_puts(fn_buf, STR_FROM(":\n"));
}

void put_r(buf *buffer, const char *op, reg_t reg) {
    buf_snprintf(buffer, "\t%s ", op);
    buf_putreg(buffer, reg);
    buf_putc(buffer, '\n');
}

void emit_fn_prologue_epilogue(const parser_context *context) {
    size_t stack_size = 0;
    const int shadow_size = 32;
    bool calls_fn = context->calls_fn;
    if (calls_fn) {
        stack_size += shadow_size; // for shadow space (x64 abi)
    }
    size_t locals_size = (size_t)context->stack_size;
    stack_size += locals_size;
    stack_size = ALIGN_TO(stack_size, (size_t)0x10);

    if (locals_size)
        buf_puts(prologue_buf, STR_FROM_INSTR("push rbp"));
    int regs_to_save = context->nreg_count;
    int tmp = regs_to_save + (locals_size ? 1 : 0);
    if (calls_fn && tmp % 2 == 0)
        stack_size += 8; // for aligning stack to 0x10 bytes on 'call' (x64 abi)

    for (int i = 0; i < regs_to_save; ++i) {
        put_r(prologue_buf, "push", (reg_t){.reg_type = NREG, .offset = i, .rsize = sizeof (void *)});
    }
    if (locals_size)
        buf_snprintf(prologue_buf, "\tlea rbp, [rsp - 0x%x]\n", shadow_size);
    if (stack_size)
        buf_snprintf(prologue_buf, "\tsub rsp, 0x%zx\n", stack_size);


    if (stack_size)
        buf_snprintf(fn_buf, "\tadd rsp, 0x%zx\n", stack_size);
    if (locals_size)
        buf_puts(fn_buf, STR_FROM_INSTR("pop rbp"));
    for (int i = 0; i < regs_to_save; ++i) {
        put_r(fn_buf, "pop", (reg_t){.reg_type = NREG, .offset = i, .rsize = sizeof (void *)});
    }
}

void emit_fn_call(const str *s) {
    buf_puts(fn_buf, STR("\tcall "));
    buf_puts(fn_buf, *s);
    buf_putc(fn_buf, '\n');
}

void emit_fn(str fn_name) {
    buf_puts(fn_header_buf, STR_FROM("\n\t.def "));
	buf_puts(fn_header_buf, fn_name);
    buf_puts(fn_header_buf, STR_FROM(";\n"));
    buf_puts(fn_header_buf, STR_FROM("\t.scl 2;\n"));
    buf_puts(fn_header_buf, STR_FROM("\t.type 32;\n"));
    buf_puts(fn_header_buf, STR_FROM("\t.endef\n"));

    buf_puts(fn_header_buf, STR_FROM("\t.globl "));
	buf_puts(fn_header_buf, fn_name);
    buf_puts(fn_header_buf, STR_FROM("\n\t.p2align 4\n"));
	buf_puts(fn_header_buf, fn_name);
    buf_puts(fn_header_buf, STR_FROM(":\n"));
}

void emit_ret(void) {
    buf_puts(fn_buf, STR_FROM_INSTR("ret"));
}


void report_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, CSI_RED"error: "CSI_RESET);
    vfprintf(stderr, format, args);
    va_end(args);
}

extern const char *text_section_header;
extern const char *string_section_header;

