#include <assert.h>
#include <inttypes.h>

#ifndef _WIN32
#include <execinfo.h>
#include <unistd.h>
#endif

#include "buffer.h"
#include "emit.h"
#include "err.h"
#include "typesys.h"

#define INIT_BUFSIZ 0x400

#define INSTR(s) "\t"s"\n"
#define STR_FROM_INSTR(s) STR(INSTR(s))

static buf text_buf;

static buf cstr_buf;
static char *cstr_begin;
static unsigned string_lit_counts;

static buf *fn_buf;

#define MAX_CONTEXTS 10
emit_context_t *contexts[MAX_CONTEXTS];
emit_context_t **contexts_top = contexts;
emit_context_t *context;

void emit_init(void) {
    buf_init(&text_buf, INIT_BUFSIZ);
    buf_puts(&text_buf, STR(text_section_header));

    buf_init(&cstr_buf, INIT_BUFSIZ);
    buf_puts(&cstr_buf, STR(string_section_header));
    cstr_begin = cstr_buf.cur;
}

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
    assert(out != NULL);

    buf_fwrite(&emit_ctx->fn_header_buf, out);
    buf_fwrite(&emit_ctx->prologue_buf, out);
    buf_fwrite(&emit_ctx->fn_buf, out);

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

static void buf_putreg(buf *buffer, reg_t reg);

static void buf_comma(buf *buffer) {
    buf_puts(buffer, STR(", "));
}

extern const char *imm_prefix;

static void buf_puti(buf *buffer, i64 i0) {
    buf_snprintf(buffer, "%s%"PRIx64, imm_prefix, (u64)i0);
}

void emit_r(buf *buffer, const char *op, reg_t reg) {
    buf_snprintf(buffer, "\t%s ", op);
    buf_putreg(buffer, reg);
    buf_putc(buffer, '\n');
}

static void emit_rx(str op, reg_t r0) {
    buf_putc(fn_buf, '\t');
    buf_puts(fn_buf, op);
    buf_putc(fn_buf, ' ');
    buf_putreg(fn_buf, r0);
}

static void emit_rix(str op, reg_t r0, i64 i0) {
    emit_rx(op, r0);
    buf_puts(fn_buf, STR(", "));
    buf_puti(fn_buf, i0);
}

static void emit_ri(str op, reg_t r0, i64 i0) {
    emit_rx(op, r0);
    buf_puts(fn_buf, STR(", "));
    buf_puti(fn_buf, i0);
    buf_putc(fn_buf, '\n');
}

static void emit_risi(str op, reg_t r0, i64 i0, str s0, i64 i1) {
    emit_rix(op, r0, i0);
    buf_comma(fn_buf);
    buf_puts(fn_buf, s0);
    buf_putc(fn_buf, ' ');
    buf_puti(fn_buf, i1);
    buf_putc(fn_buf, '\n');
}

static void emit_rrx(str op, reg_t r0, reg_t r1) {
    emit_rx(op, r0);
    buf_puts(fn_buf, STR(", "));
    buf_putreg(fn_buf, r1);
}

static void emit_rr(str op, reg_t r0, reg_t r1) {
    emit_rrx(op, r0, r1);
    buf_putc(fn_buf, '\n');
}

static void emit_rrr(str op, reg_t r0, reg_t r1, reg_t r2) {
    emit_rrx(op, r0, r1);
    buf_puts(fn_buf, STR(", "));
    buf_putreg(fn_buf, r2);
    buf_putc(fn_buf, '\n');
}

static void emit_rri(str op, reg_t r0, reg_t r1, i64 i0) {
    emit_rrx(op, r0, r1);
    buf_puts(fn_buf, STR(", "));
    buf_puti(fn_buf, i0);
    buf_putc(fn_buf, '\n');
}

static void emit_rrii(str op, reg_t r0, reg_t r1, i64 i0, i64 i1) {
    emit_rrx(op, r0, r1);
    buf_puts(fn_buf, STR(", "));
    buf_puti(fn_buf, i0);
    buf_puts(fn_buf, STR(", "));
    buf_puti(fn_buf, i1);
    buf_putc(fn_buf, '\n');
}

static void emit_rrrsi(str op, reg_t r0, reg_t r1, reg_t r2, str s, i64 i0) {
    emit_rrx(op, r0, r1);
    buf_comma(fn_buf);
    buf_putreg(fn_buf, r2);
    buf_comma(fn_buf);
    buf_puts(fn_buf, s);
    buf_comma(fn_buf);
    buf_puti(fn_buf, i0);
    buf_putc(fn_buf, '\n');
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
