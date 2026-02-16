#include <inttypes.h>
#include "buffer.h"

static buf *fn_buf;
static void buf_putreg(buf *buffer, reg_t reg);

static void buf_comma(buf *buffer) {
    buf_puts(buffer, STR_FROM(", "));
}

static void buf_puti(buf *buffer, i64 i0) {
    buf_snprintf(buffer, "0x%"PRIx64, i0);
}

static void emit_rx(str op, reg_t r0) {
    buf_putc(fn_buf, '\t');
    buf_puts(fn_buf, op);
    buf_putc(fn_buf, ' ');
    buf_putreg(fn_buf, r0);
}

static void emit_rix(str op, reg_t r0, i64 i0) {
    emit_rx(op, r0);
    buf_puts(fn_buf, STR_FROM(", "));
    buf_puti(fn_buf, i0);
}

static void emit_ri(str op, reg_t r0, i64 i0) {
    emit_rx(op, r0);
    buf_puts(fn_buf, STR_FROM(", "));
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
    buf_puts(fn_buf, STR_FROM(", "));
    buf_putreg(fn_buf, r1);
}

static void emit_rr(str op, reg_t r0, reg_t r1) {
    emit_rrx(op, r0, r1);
    buf_putc(fn_buf, '\n');
}

static void emit_rrr(str op, reg_t r0, reg_t r1, reg_t r2) {
    emit_rrx(op, r0, r1);
    buf_puts(fn_buf, STR_FROM(", "));
    buf_putreg(fn_buf, r2);
    buf_putc(fn_buf, '\n');
}

static void emit_rri(str op, reg_t r0, reg_t r1, i64 i0) {
    emit_rrx(op, r0, r1);
    buf_puts(fn_buf, STR_FROM(", "));
    buf_puti(fn_buf, i0);
    buf_putc(fn_buf, '\n');
}

static void emit_rrii(str op, reg_t r0, reg_t r1, i64 i0, i64 i1) {
    emit_rrx(op, r0, r1);
    buf_puts(fn_buf, STR_FROM(", "));
    buf_puti(fn_buf, i0);
    buf_puts(fn_buf, STR_FROM(", "));
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
