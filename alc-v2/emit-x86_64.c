#include "emit.h"
#include "buffer.h"

#define DECL_PTR(T, X) T _##X; T *X = &_##X

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
		report_error("unexpected negative register offset %d", reg.offset);
	}
	size_t offset = (size_t)reg.offset;

	const char *rname_original;
#define RNAME_START if (false)
#define RNAME(RTYPE, rtype) \
	} else if (reg_type == RTYPE) { \
		if (offset >= rname_##rtype##_len) { \
			report_error("used up all "#rtype" registers\n"); \
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
	char rname_arr[8];
	char *rname = rname_arr + 1;
	strncpy(rname, rname_original, rname_len + 1);
	reg_size rsize = reg.rsize;
	char *rname_w_size;
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
	printf("rname: %s", rname);
	buf_snprintf(buffer, "%s", rname);
}

void emit_mov(reg_t dst, i64 value) {
	buf_putreg(fn_buf, dst);
}

void emit_mov_reg(reg_t dst, reg_t src);
void emit_add(reg_t dst, reg_t lhs, i64 rhs);
void emit_add_reg(reg_t dst, reg_t lhs, reg_t rhs);
void emit_sub(reg_t dst, reg_t lhs, i64 rhs);
void emit_sub_reg(reg_t dst, reg_t lhs, reg_t rhs);
void emit_cmp(reg_t lhs, i64 rhs);
void emit_cmp_reg(reg_t lhs, reg_t rhs);
void emit_string_lit(reg_t dst, const str *s);
void emit_lsl(reg_t dst, reg_t lhs, i64 rhs);

void emit_str(reg_t src, reg_t dst, int offset);
void emit_ldr(reg_t dst, reg_t src, int offset);
void emit_str_reg(reg_t src, reg_t dst, reg_t offset);
void emit_ldr_reg(reg_t dst, reg_t src, reg_t offset);

void emit_branch(str fn_name, str label, int index);
bool emit_branch_cond(cond condition, str fn_name, str label, int index);
void emit_label(str fn_name, str label, int index);
void emit_fn_prologue_epilogue(const parser_context *context);
void emit_fn_call(const str *s);
void emit_fn(str fn_name);
void emit_ret(void);

void report_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, CSI_RED"error: "CSI_RESET);
    vfprintf(stderr, format, args);
    va_end(args);
}

extern const char *text_section_header;
extern const char *string_section_header;

