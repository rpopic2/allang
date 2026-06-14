#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

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
#define LABEL_CAP 1024
#define FIXUP_CAP 1024
#define KEY_CAP 96
#define CALLEE_START 19

#define OP_MOVZ           0x52800000u
#define OP_MOVK           0x72800000u
#define OP_ORR_REG        0x2a000000u
#define OP_MOV_REG        (OP_ORR_REG | 0x3e0u)
#define OP_ADD_IMM        0x11000000u
#define OP_SUB_IMM        0x51000000u
#define OP_SUBS_IMM       0x71000000u
#define ADDSUB_NEG_BIT    0x40000000u
#define OP_ADD_REG        0x0b000000u
#define OP_SUB_REG        0x4b000000u
#define OP_SUBS_REG       0x6b000000u
#define ADDSUB_EXT_BIT    0x00200000u
#define OP_UBFM32         0x53000000u
#define OP_UBFM64         0xd3400000u
#define OP_BFM32          0x33000000u
#define OP_BFM64          0xb3400000u
#define OP_SBFM64         0x93400000u
#define OP_AND_IMM32      0x12000000u
#define OP_AND_IMM64      0x92400000u
#define OP_CSINC          0x1a9f07e0u
#define OP_SMULL          0x9b207c00u
#define OP_LDST_USCALED   0x39000000u
#define OP_LDST_UNSCALED  0x38000000u
#define OP_LDST_REGOFF    0x38206800u
#define LDST_LOAD         0x00400000u
#define LDST_LOAD_SIGNED  0x00800000u
#define LDST_REGOFF_SCALE 0x00001000u
#define OP_SUB_SP_IMM     0xd1000000u
#define OP_ADD_SP_IMM     0x91000000u
#define OP_STP_OFF        0xa9000000u
#define OP_LDP_OFF        0xa9400000u
#define OP_STP_PRE        0xa9800000u
#define OP_LDP_POST       0xa8c00000u
#define OP_STR_PRE        0xf8000c00u
#define OP_LDR_POST       0xf8400400u
#define OP_STR_OFF64      0xf9000000u
#define OP_LDR_OFF64      0xf9400000u
#define OP_B              0x14000000u
#define OP_BCOND          0x54000000u
#define OP_BL             0x94000000u
#define OP_ADR            0x10000000u
#define OP_RET            0xd65f03c0u

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
    bool done;
} label_t;

enum fixup_kind { FX_BRANCH, FX_COND, FX_CALL, FX_STR };

typedef struct {
    char key[KEY_CAP];
    uint32_t site;
    uint32_t aux;
    uint8_t kind;
    bool done;
} fixup_t;

static label_t labels[LABEL_CAP];
static uint32_t nlabels;
static fixup_t fixups[FIXUP_CAP];
static uint32_t nfixups;

static label_t fn_syms[LABEL_CAP];
static uint32_t nfnsyms;

// A nested compile() (from #compile_all) can run between an outer function's
// emit_fn and its finalize, so per-function emit state is kept on a stack
// pushed by emit_reset_fn and popped by emit_finalize_fnbuf.
typedef struct {
    char name[KEY_CAP];
    bool is_main;
    bool named;
} fnframe_t;

static fnframe_t fnframes[32];
static int fn_depth;

static uint8_t cstrs[0x1000];
static uint32_t cstrs_len;

static bin_import imports[64];
static uint32_t n_imports;
static bin_extcall extcalls[FIXUP_CAP];
static uint32_t n_extcalls;

static const reg_t FP = { .reg_type = FRAME, .rsize = sizeof (void *) };
static const reg_t XZR = { .reg_type = RD_NONE, .rsize = 8 };
static const reg_t WZR = { .reg_type = RD_NONE, .rsize = 4 };

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

static int fnframe_cap(void) {
    return (int)(sizeof fnframes / sizeof fnframes[0]);
}

void emit_reset_fn(emit_context_t *context) {
    body_len = 0;
    prol_len = 0;
    if (fn_depth >= 0 && fn_depth < fnframe_cap()) {
        fnframes[fn_depth].named = false;
        fnframes[fn_depth].is_main = false;
        fnframes[fn_depth].name[0] = '\0';
    }
    fn_depth++;
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

    for (uint32_t i = 0; i < nlabels; i++) {
        if (!labels[i].done) {
            labels[i].word += delta;
            labels[i].done = true;
        }
    }
    for (uint32_t i = 0; i < nfixups; i++) {
        if (!fixups[i].done) {
            fixups[i].site += delta;
            fixups[i].done = true;
        }
    }

    if (fn_depth > 0)
        fn_depth--;
    if (fn_depth >= 0 && fn_depth < fnframe_cap()) {
        const fnframe_t *f = &fnframes[fn_depth];
        if (f->named) {
            if (nfnsyms < LABEL_CAP) {
                memcpy(fn_syms[nfnsyms].key, f->name, KEY_CAP);
                fn_syms[nfnsyms].word = base;
                nfnsyms++;
            }
            if (f->is_main)
                entry_off = base * (uint32_t)sizeof code[0];
        }
    }

    body_len = 0;
    prol_len = 0;
}

static bool find_word(const label_t *tab, uint32_t n, const char *key, uint32_t *out) {
    for (uint32_t j = 0; j < n; j++) {
        if (strcmp(tab[j].key, key) == 0) {
            *out = tab[j].word;
            return true;
        }
    }
    return false;
}

void bin_emit(bin_image *image) {
    static uint8_t image_buf[CODE_CAP * sizeof code[0] + sizeof cstrs];
    uint32_t code_bytes = code_len * (uint32_t)sizeof code[0];

    for (uint32_t i = 0; i < nfixups; i++) {
        const fixup_t *fx = &fixups[i];
        uint32_t target = 0;
        if (fx->kind == FX_STR) {
            int32_t rel = (int32_t)(code_bytes + fx->aux) - (int32_t)(fx->site * 4);
            code[fx->site] |= (((uint32_t)rel & 3u) << 29) | ((((uint32_t)rel >> 2) & 0x7ffffu) << 5);
            continue;
        }
        if (fx->kind == FX_CALL) {
            if (!find_word(fn_syms, nfnsyms, fx->key, &target)) {
                uint32_t imp = n_imports;
                for (uint32_t k = 0; k < n_imports; k++) {
                    if (strcmp(imports[k].name, fx->key) == 0) {
                        imp = k;
                        break;
                    }
                }
                if (imp == n_imports)
                    imports[n_imports++].name = fx->key;
                extcalls[n_extcalls].site = fx->site * (uint32_t)sizeof code[0];
                extcalls[n_extcalls].import = imp;
                n_extcalls++;
                continue;
            }
        } else if (!find_word(labels, nlabels, fx->key, &target)) {
            report_error("aarch64-bin: unresolved label '%s'\n", fx->key);
            continue;
        }
        int32_t rel = (int32_t)target - (int32_t)fx->site;
        if (fx->kind == FX_COND) {
            code[fx->site] |= ((uint32_t)(rel & 0x7ffff)) << 5;
        } else {
            code[fx->site] |= (uint32_t)(rel & 0x3ffffff);
        }
    }

    memcpy(image_buf, code, code_bytes);
    memcpy(image_buf + code_bytes, cstrs, cstrs_len);
    image->text = image_buf;
    image->text_size = code_bytes + cstrs_len;
    image->entry = entry_off;
    image->imports = imports;
    image->imports_count = n_imports;
    image->extcalls = extcalls;
    image->extcalls_count = n_extcalls;
}

bool emit_need_escaping(void) {
    return true;
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
        uint32_t base = initialized ? OP_MOVK : OP_MOVZ;
        put_word((sf << 31) | base | ((uint32_t)lane << 21) | (chunk << 5) | rd);
        initialized = true;
    }
    if (!initialized)
        put_word((sf << 31) | OP_MOVZ | rd);
}

static void emit_and_lowbits(reg_t dst, reg_t src, uint32_t nbits) {
    uint32_t base = is64(dst) ? OP_AND_IMM64 : OP_AND_IMM32;
    put_word(base | ((nbits - 1u) << 10) | (reg_no(src) << 5) | reg_no(dst));
}

void emit_mov_reg(reg_t dst, reg_t src) {
    if (reg_eq(dst, src))
        return;
    uint32_t dsize = dtype_tryget_addr(&dst.dtype) ? 8 : dst.rsize;
    const type_t *st = src.dtype.base;
    if (dsize > 4 && src.rsize <= 4 && st) {
        if (st->sign) {
            uint32_t imms = (uint32_t)(st->size * 8 - 1);
            put_word(OP_SBFM64 | (imms << 10) | (reg_no(src) << 5) | reg_no(dst));
            return;
        }
        if (st->size < 4) {
            emit_and_lowbits(dst, src, (uint32_t)(st->size * 8));
            return;
        }
        put_word(OP_ORR_REG | (reg_no(src) << 16) | 0x3e0u | reg_no(dst));
        return;
    }
    uint32_t sf = dsize > 4;
    put_word((sf << 31) | OP_MOV_REG | (reg_no(src) << 16) | reg_no(dst));
}

static void emit_orr_reg(reg_t dst, reg_t lhs, reg_t rhs) {
    uint32_t sf = is64(dst);
    put_word((sf << 31) | OP_ORR_REG | (reg_no(rhs) << 16) | (reg_no(lhs) << 5) | reg_no(dst));
}

void emit_zero_out(reg_t dst) {
    emit_mov(dst, 0);
}

static void addsub_imm(uint32_t base, reg_t dst, reg_t lhs, i64 rhs) {
    uint32_t sf = is64(dst);
    uint32_t sh = 0;
    uint64_t imm = (uint64_t)(rhs < 0 ? -rhs : rhs);
    if (rhs < 0)
        base ^= ADDSUB_NEG_BIT;
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
    addsub_imm(OP_ADD_IMM, dst, lhs, rhs);
}

void emit_sub(reg_t dst, reg_t lhs, i64 rhs) {
    if (rhs == 0) {
        emit_mov_reg(dst, lhs);
        return;
    }
    addsub_imm(OP_SUB_IMM, dst, lhs, rhs);
}

void emit_cmp(reg_t lhs, i64 rhs) {
    reg_t zr = { .reg_type = RD_NONE, .rsize = lhs.rsize };
    addsub_imm(OP_SUBS_IMM, zr, lhs, rhs);
}

static void addsub_reg(uint32_t base, reg_t dst, reg_t lhs, reg_t rhs) {
    uint32_t sf = is64(dst);
    put_word((sf << 31) | base | (reg_no(rhs) << 16)
             | (reg_no(lhs) << 5) | reg_no(dst));
}

static uint32_t ext_option(reg_t narrow) {
    const type_t *t = narrow.dtype.base;
    size_t size = t ? t->size : narrow.rsize;
    uint32_t log2 = size >= 8 ? 3u : size >= 4 ? 2u : size >= 2 ? 1u : 0u;
    uint32_t sign = (t && t->sign) ? 4u : 0u;
    return sign | log2;
}

static void addsub_ext(uint32_t plain_base, reg_t dst, reg_t wide, reg_t narrow, uint32_t imm3) {
    uint32_t sf = is64(wide);
    uint32_t base = plain_base | ADDSUB_EXT_BIT | (sf << 31);
    put_word(base | (reg_no(narrow) << 16) | (ext_option(narrow) << 13)
             | (imm3 << 10) | (reg_no(wide) << 5) | reg_no(dst));
}

void emit_add_reg(reg_t dst, reg_t lhs, reg_t rhs) {
    if (lhs.rsize != rhs.rsize) {
        reg_t wide = lhs.rsize > rhs.rsize ? lhs : rhs;
        reg_t narrow = lhs.rsize > rhs.rsize ? rhs : lhs;
        addsub_ext(OP_ADD_REG, dst, wide, narrow, 0);
        return;
    }
    addsub_reg(OP_ADD_REG, dst, lhs, rhs);
}

void emit_sub_reg(reg_t dst, reg_t lhs, reg_t rhs) {
    if (lhs.rsize == rhs.rsize) {
        addsub_reg(OP_SUB_REG, dst, lhs, rhs);
        return;
    }
    if (rhs.rsize < lhs.rsize) {
        addsub_ext(OP_SUB_REG, dst, lhs, rhs, 0);
        return;
    }
    reg_t tmp = { .reg_type = SCRATCH, .offset = 8, .rsize = dst.rsize, .dtype = lhs.dtype };
    emit_mov_reg(tmp, lhs);
    addsub_reg(OP_SUB_REG, dst, tmp, rhs);
}

void emit_cmp_reg(reg_t lhs, reg_t rhs, cond_t cond) {
    reg_t zr = { .reg_type = RD_NONE, .rsize = lhs.rsize };
    if (lhs.rsize == rhs.rsize) {
        addsub_reg(OP_SUBS_REG, zr, lhs, rhs);
        return;
    }
    if (rhs.rsize < lhs.rsize) {
        addsub_ext(OP_SUBS_REG, zr, lhs, rhs, 0);
        return;
    }
    reg_t tmp = { .reg_type = SCRATCH, .offset = 8, .rsize = rhs.rsize, .dtype = lhs.dtype };
    emit_mov_reg(tmp, lhs);
    zr.rsize = rhs.rsize;
    addsub_reg(OP_SUBS_REG, zr, tmp, rhs);
}

void emit_lsl(reg_t dst, reg_t lhs, i64 rhs) {
    uint32_t sf = is64(dst);
    uint32_t width = sf ? 64u : 32u;
    uint32_t shift = (uint32_t)rhs & (width - 1u);
    uint32_t immr = (width - shift) & (width - 1u);
    uint32_t imms = width - 1u - shift;
    uint32_t base = sf ? OP_UBFM64 : OP_UBFM32;
    put_word(base | (immr << 16) | (imms << 10) | (reg_no(lhs) << 5) | reg_no(dst));
}

static void emit_lsr_imm(reg_t dst, reg_t src, i64 rhs) {
    uint32_t sf = is64(dst);
    uint32_t width = sf ? 64u : 32u;
    uint32_t immr = (uint32_t)rhs & (width - 1u);
    uint32_t imms = width - 1u;
    uint32_t base = sf ? OP_UBFM64 : OP_UBFM32;
    put_word(base | (immr << 16) | (imms << 10) | (reg_no(src) << 5) | reg_no(dst));
}

static void emit_bitfield(uint32_t base32, uint32_t base64, reg_t dst, reg_t src, i64 lsb, i64 width) {
    uint32_t sf = is64(dst);
    uint32_t rbits = sf ? 64u : 32u;
    uint32_t base = sf ? base64 : base32;
    uint32_t immr = (uint32_t)((rbits - (uint32_t)lsb) & (rbits - 1u));
    uint32_t imms = (uint32_t)(width - 1);
    put_word(base | (immr << 16) | (imms << 10) | (reg_no(src) << 5) | reg_no(dst));
}

void emit_cond_set(reg_t dst, cond_t cond) {
    uint32_t inv = cond_aarch64[cond] ^ 1u;
    put_word(OP_CSINC | (inv << 12) | reg_no(dst));
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

static uint32_t ldst_load_opc(bool is_load, reg_t value, uint32_t sz) {
    if (!is_load)
        return 0;
    if (value.dtype.base && value.dtype.base->sign && sz < 3)
        return LDST_LOAD_SIGNED;
    return LDST_LOAD;
}

static void mem_op(bool is_load, reg_t value, reg_t base, i64 offset) {
    uint32_t sz = ldst_size_bits(value);
    int32_t access = 1 << sz;
    uint32_t rt = reg_no(value);
    uint32_t rn = reg_no(base);
    uint32_t opc = ldst_load_opc(is_load, value, sz);
    if (offset >= 0 && offset % access == 0 && offset / access <= 0xfff) {
        uint32_t imm12 = (uint32_t)(offset / access);
        put_word((sz << 30) | OP_LDST_USCALED | opc | (imm12 << 10) | (rn << 5) | rt);
    } else {
        uint32_t imm9 = (uint32_t)(offset & 0x1ff);
        put_word((sz << 30) | OP_LDST_UNSCALED | opc | (imm9 << 12) | (rn << 5) | rt);
    }
}

static void ldst_regoff(bool is_load, reg_t value, reg_t base, reg_t off, bool scaled) {
    uint32_t sz = ldst_size_bits(value);
    uint32_t opc = ldst_load_opc(is_load, value, sz);
    uint32_t s = scaled ? LDST_REGOFF_SCALE : 0u;
    put_word((sz << 30) | OP_LDST_REGOFF | opc | (reg_no(off) << 16) | s
             | (reg_no(base) << 5) | reg_no(value));
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

void emit_str_regoff(reg_t dst, reg_t src, reg_t offset) {
    ldst_regoff(false, src, dst, offset, false);
}

void emit_ldr_reg(reg_t dst, reg_t src, reg_t offset) {
    ldst_regoff(true, dst, src, offset, false);
}

void emit_str_imm_regoff(reg_t dst, i64 value, reg_t offset) {
    reg_t tmp = {
        .reg_type = SCRATCH, .offset = 8,
        .rsize = dst.dtype.base ? (reg_size)dst.dtype.base->size : 8,
        .dtype = { .base = dst.dtype.base },
    };
    emit_mov(tmp, value);
    emit_str_regoff(dst, tmp, offset);
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
        put_prologue(addsub_imm64(OP_SUB_SP_IMM, 31, 31, stack_size));

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
            put_prologue(stp_word(OP_STP_PRE, reg0, reg1, 31, -stack_size));
            defer_ldp = true;
            deferred0 = (int)reg0;
            deferred1 = (int)reg1;
        } else {
            put_prologue(stp_word(OP_STP_OFF, reg0, reg1, 31, off));
            put_word(stp_word(OP_LDP_OFF, reg0, reg1, 31, off));
        }
        remaining -= 2;
        cur_stackoff += 16;
    }
    while (remaining > 1) {
        uint32_t reg0 = (uint32_t)(CALLEE_START + remaining - 1);
        uint32_t reg1 = reg0 - 1;
        put_prologue(stp_word(OP_STP_OFF, reg0, reg1, 31, cur_stackoff));
        put_word(stp_word(OP_LDP_OFF, reg0, reg1, 31, cur_stackoff));
        remaining -= 2;
        cur_stackoff += 16;
    }
    if (remaining == 1) {
        remaining -= 1;
        uint32_t reg0 = (uint32_t)(CALLEE_START + remaining);
        if (cur_stackoff == 0) {
            put_prologue(str_imm9_word(OP_STR_PRE, reg0, 31, -stack_size));
            put_word(str_imm9_word(OP_LDR_POST, reg0, 31, stack_size));
        } else {
            put_prologue(str_off64_word(OP_STR_OFF64, reg0, 31, cur_stackoff));
            put_word(str_off64_word(OP_LDR_OFF64, reg0, 31, cur_stackoff));
        }
        cur_stackoff += 16;
    }

    if (needs_fp)
        put_prologue(addsub_imm64(OP_ADD_SP_IMM, 29, 31, stack_objs_size));

    if (defer_ldp)
        put_word(stp_word(OP_LDP_POST, (uint32_t)deferred0, (uint32_t)deferred1, 31, stack_size));
    if (stack_objs_size > 0)
        put_word(addsub_imm64(OP_ADD_SP_IMM, 31, 31, stack_size));
}

void emit_branch(str fn_name, str label, int index) {
    if (nfixups >= FIXUP_CAP) {
        report_error("aarch64-bin: fixup overflow\n");
        return;
    }
    fixup_t *fx = &fixups[nfixups++];
    make_key(fx->key, fn_name, label, index);
    fx->site = body_len;
    fx->kind = FX_BRANCH;
    put_word(OP_B);
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
    fx->kind = FX_COND;
    put_word(OP_BCOND | cond_aarch64[condition]);
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
    if (fn_depth > 0 && fn_depth <= fnframe_cap()) {
        fnframe_t *f = &fnframes[fn_depth - 1];
        snprintf(f->name, KEY_CAP, "%.*s", (int)str_len(fn_name), fn_name.data);
        f->is_main = str_eq(fn_name, STR("main"));
        f->named = true;
    }
}

void emit_ret(void) {
    put_word(OP_RET);
}


static void emit_stp(reg_t src1, reg_t src2, reg_t base, i64 offset) {
    put_word(stp_word(OP_STP_OFF, reg_no(src1), reg_no(src2), reg_no(base), (int)offset));
}

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
    uint32_t sf = is64(dst);
    uint32_t lane = (uint32_t)(offset_bits / 16);
    uint32_t base = *initialized ? OP_MOVK : OP_MOVZ;
    put_word((sf << 31) | base | (lane << 21) | (((uint32_t)chunk & 0xffff) << 5) | reg_no(dst));
    *initialized = true;
}

static size_t emit_member_value(reg_t dst, const dyn_member_t *members, const dyn_agg_member *args,
                                ptrdiff_t *i, ptrdiff_t member_count,
                                size_t memb_size, size_t offset_bits,
                                bool *dst_initialized) {
    i64 value = args->begin[*i].value;
    size_t packed = memb_size;
    if (offset_bits >= 32 && dst.rsize < 8)
        dst.rsize = 8;
    if (offset_bits % 16 == 0)
        packed = pack_small_values(members, args, i, member_count, memb_size, &value);
    if (value == 0)
        return packed;

    if (offset_bits % 16 != 0) {
        reg_t tmp = { .reg_type = SCRATCH, .offset = 8, .rsize = dst.rsize };
        emit_mov(tmp, value << offset_bits);
        if (!*dst_initialized) {
            emit_mov_reg(dst, tmp);
            *dst_initialized = true;
        } else {
            emit_orr_reg(dst, dst, tmp);
        }
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
            if (reg.rsize < dst.rsize)
                reg.rsize = dst.rsize;
            emit_and_lowbits(dst, reg, (uint32_t)(memb_size * 8));
        } else {
            reg_t tmp_dst = dst;
            if (reg.rsize < tmp_dst.rsize)
                tmp_dst.rsize = reg.rsize;
            tmp_dst.rsize = (u8)memb_size;
            emit_mov_reg(tmp_dst, reg);
        }
    } else {
        if (reg.rsize < dst.rsize)
            reg.rsize = dst.rsize;
        i64 width = (i64)memb_size * 8;
        if (dst_initialized)
            emit_bitfield(OP_BFM32, OP_BFM64, dst, reg, (i64)offset_bits, width);
        else
            emit_bitfield(OP_UBFM32, OP_UBFM64, dst, reg, (i64)offset_bits, width);
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
            ? &(member_t){ .dtype = *dtype, .offset = (size_t)i * dtype->base->size }
            : &members->begin[i];
        size_t memb_size = is_arr ? dtype->base->size : dtype_size(&memb->dtype);
        size_t offset_bits = memb->offset * 8;

        if (size_acc == 0)
            base_offset_bits = offset_bits;
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
            report_error("aarch64-bin: unexpected aggregate member tag\n");
            break;
        }
    }

    *size += size_acc;
    return dst_initialized;
}

void emit_store_eightbytes(reg_t base, i64 offset, reg_t lo, bool lo_written,
                           reg_t hi, bool hi_written, bool has_hi) {
    if (has_hi) {
        lo.rsize = 8;
        hi.rsize = 8;
        if (!lo_written)
            lo = XZR;
        if (!hi_written)
            hi = XZR;
        emit_stp(lo, hi, base, offset);
        return;
    }
    if (!lo_written)
        lo.reg_type = RD_NONE;
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
            emit_lsr_imm(full, full, (i64)chunk * 8);
        }
    }
}

void emit_zerofill(reg_t dst, i64 offset, const dtype_t *type) {
    size_t size = dtype_size(type);

    while (size >= 16) {
        emit_stp(XZR, XZR, dst, offset);
        size -= 16;
        offset += 16;
    }
    while (size >= 8) {
        emit_str_reg(dst, XZR, (int)offset);
        size -= 8;
        offset += 8;
    }

    reg_t zr = WZR;
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

void emit_make_array(reg_t dst, type_t *type, u32 len, dyn_regable *args) {}
void emit_store_array(reg_t dst, i64 offset, type_t *type, u32 len, dyn_regable *args) {}

void emit_array_access(reg_t dst, reg_t src, reg_t offset, load_store_t is_store) {
    dtype_t *dtype = &src.dtype;
    size_t array_size = dtype_size(dtype);
    size_t elem_size = dtype->base->size;

    if (src.reg_type == STACK && src.offset) {
        reg_t tmp_src = src;
        tmp_src.reg_type = SCRATCH;
        tmp_src.offset = 3;
        tmp_src.rsize = 8;
        dtype_push(&tmp_src.dtype, (declarator_t){ .tag = DK_ADDR, .amount = 1 });
        emit_sub(tmp_src, FP, src.offset);
        src = tmp_src;
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
            ldst_regoff(!is_store, dst, src, offset, true);
            return;
        }
    }

    reg_t size;
    if (array_size) {
        size = (reg_t){ .reg_type = SCRATCH, .offset = 1 };
        emit_mov(size, (i64)array_size);
    } else {
        size = (reg_t){ .reg_type = RD_NONE };
    }
    reg_t idx = { .reg_type = SCRATCH, .offset = 2 };
    put_word(OP_SMULL | (reg_no(size) << 16) | (reg_no(idx) << 5) | reg_no(idx));

    if (is_store)
        emit_str_regoff(dst, src, offset);
    else
        emit_ldr_reg(dst, src, offset);
}

void emit_elem_addr(reg_t dst, reg_t object, reg_t index) {
    reg_t base = { .reg_type = SCRATCH, .offset = 0, .rsize = sizeof (void *) };
    emit_sub(base, FP, object.offset);

    const size_t elem_size = object.dtype.base->size;
    const uint32_t shift = (uint32_t)__builtin_ctz((unsigned)elem_size);
    const type_t *itype = index.dtype.base;

    if (itype->size >= 8) {
        index.rsize = 8;
        uint32_t sf = is64(dst);
        put_word((sf << 31) | OP_ADD_REG | (reg_no(index) << 16) | (shift << 10)
                 | (reg_no(base) << 5) | reg_no(dst));
    } else {
        index.rsize = 4;
        addsub_ext(OP_ADD_REG, dst, base, index, shift);
    }
}

void emit_string_lit(reg_t dst, const str *s) {
    const char *p = s->data;
    size_t n = str_len(*s);
    if (n >= 2 && p[0] == '"') {
        p += 1;
        n -= 2;
    }
    if (cstrs_len + n + 1 > sizeof cstrs) {
        report_error("aarch64-bin: cstring buffer overflow\n");
        return;
    }
    uint32_t str_off = cstrs_len;
    memcpy(cstrs + cstrs_len, p, n);
    cstrs_len += (uint32_t)n;
    cstrs[cstrs_len++] = '\0';

    if (nfixups >= FIXUP_CAP) {
        report_error("aarch64-bin: fixup overflow\n");
        return;
    }
    fixup_t *fx = &fixups[nfixups++];
    fx->site = body_len;
    fx->aux = str_off;
    fx->kind = FX_STR;
    put_word(OP_ADR | reg_no(dst));
}

void emit_fn_call(const str *s) {
    if (nfixups >= FIXUP_CAP) {
        report_error("aarch64-bin: fixup overflow\n");
        return;
    }
    fixup_t *fx = &fixups[nfixups++];
    snprintf(fx->key, KEY_CAP, "%.*s", (int)str_len(*s), s->data);
    fx->site = body_len;
    fx->kind = FX_CALL;
    put_word(OP_BL);
}

#pragma clang diagnostic pop

const size_t default_register_size = 8;
