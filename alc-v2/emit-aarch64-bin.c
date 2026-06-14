#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <execinfo.h>

#include "types.h"
#include "emit.h"
#include "err.h"
#include "str.h"
#include "typesys.h"
#include "emit-bin.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

#define CODE_CAP 0x2000
#define PROL_CAP 64
#define LABEL_CAP 256
#define FIXUP_CAP 256
#define KEY_CAP 96
#define CALLEE_START 19

static uint32_t code[CODE_CAP];
static uint32_t code_len;
static uint32_t entry_off;

static uint32_t body[CODE_CAP];
static uint32_t body_len;
static uint32_t prol[PROL_CAP];
static uint32_t prol_len;

typedef struct {
    char key[KEY_CAP];
    uint32_t word;
} label_t;

typedef struct {
    char key[KEY_CAP];
    uint32_t site;
    bool is_cond;
} fixup_t;

static label_t labels[LABEL_CAP];
static uint32_t nlabels;
static uint32_t fn_lbl_start;
static fixup_t fixups[FIXUP_CAP];
static uint32_t nfixups;
static uint32_t fn_fix_start;

static const uint8_t cond_aarch64[] = {
    0x0, 0x1, 0xa, 0xb, 0xc, 0xd, 0x2, 0x3, 0x8, 0x9,
};

static void put_word(uint32_t w) {
    if (body_len >= CODE_CAP) {
        report_error("aarch64-bin: code buffer overflow\n");
        return;
    }
    body[body_len++] = w;
}

static void put_prologue(uint32_t w) {
    if (prol_len >= PROL_CAP) {
        report_error("aarch64-bin: prologue buffer overflow\n");
        return;
    }
    prol[prol_len++] = w;
}

static uint32_t reg_no(reg_t r) {
    switch (r.reg_type) {
    case SCRATCH:
        return (uint32_t)(r.offset + 8);
    case NREG:
        return (uint32_t)(r.offset + 19);
    case FRAME:
        return 29;
    case STACK:
        return 31;
    case RD_NONE:
        return 31;
    default:
        return (uint32_t)r.offset;
    }
}

static uint32_t is64(reg_t r) {
    if (dtype_tryget_addr(&r.dtype))
        return 1;
    return r.rsize > 4 ? 1 : 0;
}

static void make_key(char dst[KEY_CAP], str fn_name, str label, int index) {
    snprintf(dst, KEY_CAP, "%.*s.%.*s.%d",
             (int)str_len(fn_name), fn_name.data,
             (int)str_len(label), label.data, index);
}

void emit_reset_fn(emit_context_t *context) {
    body_len = 0;
    prol_len = 0;
    fn_lbl_start = nlabels;
    fn_fix_start = nfixups;
}

void emit_finalize_fnbuf(emit_context_t *context, FILE *out) {
    uint32_t base = code_len;
    uint32_t delta = base + prol_len;
    if (code_len + prol_len + body_len > CODE_CAP) {
        report_error("aarch64-bin: code buffer overflow\n");
        return;
    }
    memcpy(code + code_len, prol, prol_len * sizeof prol[0]);
    code_len += prol_len;
    memcpy(code + code_len, body, body_len * sizeof body[0]);
    code_len += body_len;

    for (uint32_t i = fn_lbl_start; i < nlabels; i++)
        labels[i].word += delta;
    for (uint32_t i = fn_fix_start; i < nfixups; i++)
        fixups[i].site += delta;
}

void bin_emit(bin_image *image) {
    for (uint32_t i = 0; i < nfixups; i++) {
        const fixup_t *fx = &fixups[i];
        uint32_t target = 0;
        bool found = false;
        for (uint32_t j = 0; j < nlabels; j++) {
            if (strcmp(labels[j].key, fx->key) == 0) {
                target = labels[j].word;
                found = true;
                break;
            }
        }
        if (!found) {
            report_error("aarch64-bin: unresolved label '%s'\n", fx->key);
            continue;
        }
        int32_t rel = (int32_t)target - (int32_t)fx->site;
        if (fx->is_cond) {
            code[fx->site] |= ((uint32_t)(rel & 0x7ffff)) << 5;
        } else {
            code[fx->site] |= (uint32_t)(rel & 0x3ffffff);
        }
    }

    image->text = (const uint8_t *)code;
    image->text_size = code_len * sizeof code[0];
    image->entry = entry_off;
}

bool emit_need_escaping(void) {
    return false;
}

void emit_mov(reg_t dst, i64 value) {
    uint32_t sf = is64(dst);
    uint32_t rd = reg_no(dst);
    if (value > UINT32_MAX || value < 0)
        sf = 1;
    int lanes = sf ? 4 : 2;
    uint64_t uv = (uint64_t)value;
    bool initialized = false;
    for (int lane = 0; lane < lanes; lane++) {
        uint32_t chunk = (uint32_t)((uv >> (lane * 16)) & 0xffff);
        if (chunk == 0 && initialized)
            continue;
        uint32_t base = initialized ? 0x72800000u : 0x52800000u;
        put_word((sf << 31) | base | ((uint32_t)lane << 21) | (chunk << 5) | rd);
        initialized = true;
    }
    if (!initialized)
        put_word((sf << 31) | 0x52800000u | rd);
}

void emit_mov_reg(reg_t dst, reg_t src) {
    if (reg_eq(dst, src))
        return;
    uint32_t sf = is64(dst);
    put_word((sf << 31) | 0x2a0003e0u | (reg_no(src) << 16) | reg_no(dst));
}

void emit_zero_out(reg_t dst) {
    emit_mov(dst, 0);
}

static void addsub_imm(uint32_t base, reg_t dst, reg_t lhs, i64 rhs) {
    uint32_t sf = is64(dst);
    uint32_t sh = 0;
    uint64_t imm = (uint64_t)(rhs < 0 ? -rhs : rhs);
    if (rhs < 0)
        base ^= 0x40000000u;
    if (imm > 0xfff) {
        sh = 1;
        imm >>= 12;
    }
    put_word((sf << 31) | base | (sh << 22) | ((uint32_t)(imm & 0xfff) << 10)
             | (reg_no(lhs) << 5) | reg_no(dst));
}

void emit_add(reg_t dst, reg_t lhs, i64 rhs) {
    if (rhs == 0) {
        emit_mov_reg(dst, lhs);
        return;
    }
    addsub_imm(0x11000000u, dst, lhs, rhs);
}

void emit_sub(reg_t dst, reg_t lhs, i64 rhs) {
    if (rhs == 0) {
        emit_mov_reg(dst, lhs);
        return;
    }
    addsub_imm(0x51000000u, dst, lhs, rhs);
}

void emit_cmp(reg_t lhs, i64 rhs) {
    reg_t zr = { .reg_type = RD_NONE, .rsize = lhs.rsize };
    addsub_imm(0x71000000u, zr, lhs, rhs);
}

static void addsub_reg(uint32_t base, reg_t dst, reg_t lhs, reg_t rhs) {
    uint32_t sf = is64(dst);
    put_word((sf << 31) | base | (reg_no(rhs) << 16)
             | (reg_no(lhs) << 5) | reg_no(dst));
}

void emit_add_reg(reg_t dst, reg_t lhs, reg_t rhs) {
    addsub_reg(0x0b000000u, dst, lhs, rhs);
}

void emit_sub_reg(reg_t dst, reg_t lhs, reg_t rhs) {
    addsub_reg(0x4b000000u, dst, lhs, rhs);
}

void emit_cmp_reg(reg_t lhs, reg_t rhs, cond_t cond) {
    reg_t zr = { .reg_type = RD_NONE, .rsize = lhs.rsize };
    addsub_reg(0x6b000000u, zr, lhs, rhs);
}

void emit_lsl(reg_t dst, reg_t lhs, i64 rhs) {
    uint32_t sf = is64(dst);
    uint32_t width = sf ? 64u : 32u;
    uint32_t shift = (uint32_t)rhs & (width - 1u);
    uint32_t immr = (width - shift) & (width - 1u);
    uint32_t imms = width - 1u - shift;
    uint32_t base = sf ? 0xd3400000u : 0x53000000u;
    put_word(base | (immr << 16) | (imms << 10) | (reg_no(lhs) << 5) | reg_no(dst));
}

void emit_cond_set(reg_t dst, cond_t cond) {
    uint32_t inv = cond_aarch64[cond] ^ 1u;
    put_word(0x1a9f07e0u | (inv << 12) | reg_no(dst));
}

static uint32_t ldst_size_bits(reg_t r) {
    size_t sz = dtype_tryget_addr(&r.dtype) ? 8 : r.rsize;
    if (sz >= 8)
        return 3;
    if (sz >= 4)
        return 2;
    if (sz >= 2)
        return 1;
    return 0;
}

static void mem_op(bool is_load, reg_t value, reg_t base, i64 offset) {
    uint32_t sz = ldst_size_bits(value);
    int32_t access = 1 << sz;
    uint32_t rt = reg_no(value);
    uint32_t rn = reg_no(base);
    bool sign_ext = is_load && value.dtype.base && value.dtype.base->sign && sz < 3;
    uint32_t opc = is_load ? (sign_ext ? 0x00800000u : 0x00400000u) : 0u;
    if (offset >= 0 && offset % access == 0 && offset / access <= 0xfff) {
        uint32_t imm12 = (uint32_t)(offset / access);
        put_word((sz << 30) | 0x39000000u | opc | (imm12 << 10) | (rn << 5) | rt);
    } else {
        uint32_t imm9 = (uint32_t)(offset & 0x1ff);
        put_word((sz << 30) | 0x38000000u | opc | (imm9 << 12) | (rn << 5) | rt);
    }
}

void emit_str_reg(reg_t dst, reg_t src, int offset) {
    mem_op(false, src, dst, offset);
}

void emit_ldr(reg_t dst, reg_t src, int offset) {
    mem_op(true, dst, src, offset);
}

void emit_str_imm(reg_t dst, i64 value, int offset) {
    reg_t tmp = {
        .reg_type = SCRATCH, .offset = 8,
        .rsize = dst.dtype.base ? (reg_size)dst.dtype.base->size : 8,
        .dtype = { .base = dst.dtype.base },
    };
    emit_mov(tmp, value);
    emit_str_reg(dst, tmp, offset);
}

static uint32_t addsub_imm64(uint32_t base, uint32_t rd, uint32_t rn, int imm) {
    return base | (((uint32_t)imm & 0xfff) << 10) | (rn << 5) | rd;
}

static uint32_t stp_word(uint32_t base, uint32_t rt, uint32_t rt2, uint32_t rn, int imm_bytes) {
    uint32_t imm7 = (uint32_t)(imm_bytes / 8) & 0x7f;
    return base | (imm7 << 15) | (rt2 << 10) | (rn << 5) | rt;
}

static uint32_t str_imm9_word(uint32_t base, uint32_t rt, uint32_t rn, int imm_bytes) {
    return base | (((uint32_t)imm_bytes & 0x1ff) << 12) | (rn << 5) | rt;
}

static uint32_t str_off64_word(uint32_t base, uint32_t rt, uint32_t rn, int imm_bytes) {
    return base | (((uint32_t)(imm_bytes / 8) & 0xfff) << 10) | (rn << 5) | rt;
}

void emit_fn_prologue_epilogue(const parser_context *pc) {
    prol_len = 0;
    if (!pc->calls_fn && pc->max_nreg_count == 0 && pc->stack_size == 0)
        return;

    int regs_to_save = pc->max_nreg_count;
    if (regs_to_save + CALLEE_START >= 28) {
        report_error("aarch64-bin: out of callee-saved registers\n");
        return;
    }
    bool calls_fn = pc->calls_fn;
    bool needs_fp = calls_fn || pc->stack_size > 0;
    if (needs_fp)
        regs_to_save += 2;

    int stack_size = ALIGN_TO(regs_to_save * (int)sizeof(u64), 16)
                   + ALIGN_TO(pc->stack_size, 16);
    int cur_stackoff = ALIGN_TO(pc->stack_size, 16);
    const int stack_objs_size = cur_stackoff;

    if (stack_objs_size > 0)
        put_prologue(addsub_imm64(0xd1000000u, 31, 31, stack_size));

    int remaining = regs_to_save;
    bool defer_ldp = false;
    int deferred0 = 0, deferred1 = 0;

    if (remaining > 1) {
        uint32_t reg0 = (uint32_t)(CALLEE_START + remaining - 1);
        uint32_t reg1 = reg0 - 1;
        int off = cur_stackoff;
        if (needs_fp) {
            reg0 = 29;
            reg1 = 30;
        }
        if (cur_stackoff == 0) {
            put_prologue(stp_word(0xa9800000u, reg0, reg1, 31, -stack_size));
            defer_ldp = true;
            deferred0 = (int)reg0;
            deferred1 = (int)reg1;
        } else {
            put_prologue(stp_word(0xa9000000u, reg0, reg1, 31, off));
            put_word(stp_word(0xa9400000u, reg0, reg1, 31, off));
        }
        remaining -= 2;
        cur_stackoff += 16;
    }
    while (remaining > 1) {
        uint32_t reg0 = (uint32_t)(CALLEE_START + remaining - 1);
        uint32_t reg1 = reg0 - 1;
        put_prologue(stp_word(0xa9000000u, reg0, reg1, 31, cur_stackoff));
        put_word(stp_word(0xa9400000u, reg0, reg1, 31, cur_stackoff));
        remaining -= 2;
        cur_stackoff += 16;
    }
    if (remaining == 1) {
        remaining -= 1;
        uint32_t reg0 = (uint32_t)(CALLEE_START + remaining);
        if (cur_stackoff == 0) {
            put_prologue(str_imm9_word(0xf8000c00u, reg0, 31, -stack_size));
            put_word(str_imm9_word(0xf8400400u, reg0, 31, stack_size));
        } else {
            put_prologue(str_off64_word(0xf9000000u, reg0, 31, cur_stackoff));
            put_word(str_off64_word(0xf9400000u, reg0, 31, cur_stackoff));
        }
        cur_stackoff += 16;
    }

    if (needs_fp)
        put_prologue(addsub_imm64(0x91000000u, 29, 31, stack_objs_size));

    if (defer_ldp)
        put_word(stp_word(0xa8c00000u, (uint32_t)deferred0, (uint32_t)deferred1, 31, stack_size));
    if (stack_objs_size > 0)
        put_word(addsub_imm64(0x91000000u, 31, 31, stack_size));
}

void emit_branch(str fn_name, str label, int index) {
    if (nfixups >= FIXUP_CAP) {
        report_error("aarch64-bin: fixup overflow\n");
        return;
    }
    fixup_t *fx = &fixups[nfixups++];
    make_key(fx->key, fn_name, label, index);
    fx->site = body_len;
    fx->is_cond = false;
    put_word(0x14000000u);
}

bool emit_branch_cond(cond_t condition, str fn_name, str label, int index) {
    if (condition >= (cond_t)(sizeof cond_aarch64)) {
        report_error("aarch64-bin: unknown condition %d\n", condition);
        return false;
    }
    if (nfixups >= FIXUP_CAP) {
        report_error("aarch64-bin: fixup overflow\n");
        return false;
    }
    fixup_t *fx = &fixups[nfixups++];
    make_key(fx->key, fn_name, label, index);
    fx->site = body_len;
    fx->is_cond = true;
    put_word(0x54000000u | cond_aarch64[condition]);
    return true;
}

void emit_label(str fn_name, str label, int index) {
    if (nlabels >= LABEL_CAP) {
        report_error("aarch64-bin: label overflow\n");
        return;
    }
    label_t *l = &labels[nlabels++];
    make_key(l->key, fn_name, label, index);
    l->word = body_len;
}

void emit_fn(str fn_name) {
    if (str_eq(fn_name, STR("main")))
        entry_off = code_len * (uint32_t)sizeof code[0];
}

void emit_ret(void) {
    put_word(0xd65f03c0u);
}

bool emit_eightbyte_struct(reg_t dst, const dtype_t *dtype, const dyn_agg_member *args, int *index, size_t *size, size_t limit) {
    return false;
}
void emit_store_eightbytes(reg_t base, i64 offset, reg_t lo, bool lo_written, reg_t hi, bool hi_written, bool has_hi) {}
void emit_store_packed(reg_t base, i64 offset, reg_t src, size_t nbytes) {}
void emit_zerofill(reg_t dst, i64 offset, const dtype_t *type) {}

void emit_make_array(reg_t dst, type_t *type, u32 len, dyn_regable *args) {}
void emit_store_array(reg_t dst, i64 offset, type_t *type, u32 len, dyn_regable *args) {}
void emit_array_access(reg_t dst, reg_t src, reg_t offset, load_store_t is_store) {}
void emit_elem_addr(reg_t dst, reg_t object, reg_t index) {}

void emit_string_lit(reg_t dst, const str *s) {}

void emit_str_regoff(reg_t dst, reg_t src, reg_t offset) {}
void emit_str_imm_regoff(reg_t dst, i64 value, reg_t offset) {}
void emit_ldr_reg(reg_t dst, reg_t src, reg_t offset) {}

void emit_fn_call(const str *s) {}

__attribute__((format(printf, 1, 2)))
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

#pragma clang diagnostic pop

const size_t default_register_size = 8;
