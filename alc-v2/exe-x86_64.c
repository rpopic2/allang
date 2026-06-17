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

#define CODE_CAP 0x20000
#define PROL_CAP 256
#define LABEL_CAP 1024
#define FIXUP_CAP 1024
#define KEY_CAP 96
#define CSTR_CAP 0x2000
#define IMPORT_CAP 64

#define REX_BASE 0x40u
#define REX_W    0x08u
#define REX_R    0x04u
#define REX_X    0x02u
#define REX_B    0x01u

#define OP_ADD_RM_R  0x01u
#define OP_OR_RM_R   0x09u
#define OP_AND_RM_R  0x21u
#define OP_SUB_RM_R  0x29u
#define OP_XOR_RM_R  0x31u
#define OP_CMP_RM_R  0x39u
#define OP_MOV_RM_R  0x89u
#define OP_MOV_R_RM  0x8bu
#define OP_LEA       0x8du
#define OP_MOVSXD    0x63u

#define ALU_ADD 0u
#define ALU_OR  1u
#define ALU_AND 4u
#define ALU_SUB 5u
#define ALU_XOR 6u
#define ALU_CMP 7u

#define SH_SHL 4u
#define SH_SHR 5u
#define SH_SAR 7u

#define OP_GRP1_IMM8  0x83u
#define OP_GRP1_IMM32 0x81u
#define OP_GRP1_IMM8B 0x80u
#define OP_GRP_SHIFT  0xc1u
#define OP_GRP3       0xf7u
#define GRP3_NEG      3u
#define OP_MOV_IMM_RM 0xc7u
#define OP_MOV_IMM_R  0xb8u
#define OP_MOV_IMM_R8 0xb0u
#define OP_PUSH_R     0x50u
#define OP_POP_R      0x58u
#define OP_JMP_REL32  0xe9u
#define OP_CALL_REL32 0xe8u
#define OP_RET        0xc3u
#define OP_0F         0x0fu
#define OP_JCC_REL32  0x80u
#define OP_SETCC      0x90u
#define OP_MOVZX_8    0xb6u
#define OP_MOVZX_16   0xb7u
#define OP_MOVSX_8    0xbeu
#define OP_MOVSX_16   0xbfu
#define OP_IMUL_IMM8  0x6bu
#define OP_IMUL_IMM32 0x69u

static uint8_t code[CODE_CAP];
static uint32_t code_len;
static uint32_t entry_off;

static uint8_t body[CODE_CAP];
static uint32_t body_len;
static uint8_t prol[PROL_CAP];
static uint32_t prol_len;
static bool emit_to_prol;

typedef struct {
    char key[KEY_CAP];
    uint32_t at;
    bool done;
} label_t;

enum fixup_kind { FX_JMP, FX_JCC, FX_CALL, FX_STR };

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

static emit_context_t *active_ctx;

static uint8_t cstrs[CSTR_CAP];
static uint32_t cstrs_len;

static bin_import imports[IMPORT_CAP];
static uint32_t n_imports;
static bin_extcall extcalls[FIXUP_CAP];
static uint32_t n_extcalls;

extern const uint8_t scratch_regs[];
extern const uint8_t callee_regs[];
extern const uint8_t param_regs[];
extern const uint8_t ret_regs[];

static const uint8_t cond_cc[] = {
    0x4, 0x5, 0xd, 0xc, 0xf, 0xe, 0x3, 0x2, 0x7, 0x6,
};

#define RAX 0u
#define RBP 5u
#define RSP 4u

static void put8(uint8_t b) {
    if (emit_to_prol) {
        if (prol_len >= PROL_CAP) {
            report_error("x86_64-bin: prologue buffer overflow\n");
            return;
        }
        prol[prol_len++] = b;
        return;
    }
    if (body_len >= CODE_CAP) {
        report_error("x86_64-bin: code buffer overflow\n");
        return;
    }
    body[body_len++] = b;
}

static void put32(uint32_t v) {
    put8((uint8_t)(v & 0xffu));
    put8((uint8_t)((v >> 8) & 0xffu));
    put8((uint8_t)((v >> 16) & 0xffu));
    put8((uint8_t)((v >> 24) & 0xffu));
}

static void put64(uint64_t v) {
    put32((uint32_t)(v & 0xffffffffu));
    put32((uint32_t)(v >> 32));
}

static uint32_t cur_off(void) {
    return emit_to_prol ? prol_len : body_len;
}

static uint32_t reg_no(reg_t r) {
    switch (r.reg_type) {
    case SCRATCH:
        return scratch_regs[r.offset];
    case NREG:
        return callee_regs[r.offset];
    case PARAM:
        return param_regs[r.offset];
    case RET:
        return ret_regs[r.offset];
    case FRAME:
        return RBP;
    case STACK:
        return RSP;
    case RD_NONE:
        return RAX;
    default:
        return RAX;
    }
}

static reg_size rsz(reg_t r) {
    return r.rsize ? r.rsize : 4;
}

static void make_key(char dst[KEY_CAP], str fn_name, str label, int index) {
    snprintf(dst, KEY_CAP, "%.*s.%.*s.%d",
             (int)str_len(fn_name), fn_name.data,
             (int)str_len(label), label.data, index);
}

static void encode_rr(uint8_t opcode, reg_size sz, uint32_t reg, uint32_t rm) {
    if (sz == 2)
        put8(0x66);
    uint8_t rex = 0;
    if (sz == 8)
        rex |= REX_W;
    if (reg >= 8)
        rex |= REX_R;
    if (rm >= 8)
        rex |= REX_B;
    bool byte_op = (sz == 1);
    bool force_rex = byte_op && ((reg >= 4 && reg <= 7) || (rm >= 4 && rm <= 7));
    if (rex || force_rex)
        put8(REX_BASE | rex);
    put8(byte_op ? (opcode & 0xfeu) : opcode);
    put8((uint8_t)(0xc0u | ((reg & 7u) << 3) | (rm & 7u)));
}

static uint8_t choose_mod(int32_t disp, uint32_t base_low3) {
    if (disp == 0 && base_low3 != 5)
        return 0;
    if (disp >= -128 && disp <= 127)
        return 1;
    return 2;
}

static void put_disp(int32_t disp, uint8_t mod) {
    if (mod == 1)
        put8((uint8_t)(int8_t)disp);
    else if (mod == 2)
        put32((uint32_t)disp);
}

static void encode_mem(uint8_t opcode, reg_size sz, uint32_t reg_field,
                       uint32_t base, bool has_index, uint32_t index,
                       uint32_t scale, int32_t disp) {
    if (sz == 2)
        put8(0x66);
    uint8_t rex = 0;
    if (sz == 8)
        rex |= REX_W;
    if (reg_field >= 8)
        rex |= REX_R;
    if (has_index && index >= 8)
        rex |= REX_X;
    if (base >= 8)
        rex |= REX_B;
    bool byte_op = (sz == 1);
    bool force_rex = byte_op && (reg_field >= 4 && reg_field <= 7);
    if (rex || force_rex)
        put8(REX_BASE | rex);
    put8(byte_op ? (opcode & 0xfeu) : opcode);

    uint32_t base_low3 = base & 7u;
    uint8_t mod = choose_mod(disp, base_low3);
    bool need_sib = has_index || base_low3 == 4u;
    if (need_sib) {
        put8((uint8_t)(((uint32_t)mod << 6) | ((reg_field & 7u) << 3) | 4u));
        uint32_t idx3 = has_index ? (index & 7u) : 4u;
        uint32_t scale_bits = scale == 8 ? 3u : scale == 4 ? 2u : scale == 2 ? 1u : 0u;
        put8((uint8_t)((scale_bits << 6) | (idx3 << 3) | base_low3));
    } else {
        put8((uint8_t)(((uint32_t)mod << 6) | ((reg_field & 7u) << 3) | base_low3));
    }
    put_disp(disp, mod);
}

static void alu_rr(uint8_t opcode, reg_t dst, reg_t src) {
    encode_rr(opcode, rsz(dst), reg_no(src), reg_no(dst));
}

static void alu_ri(uint8_t digit, reg_t dst, i64 imm) {
    reg_size sz = rsz(dst);
    uint32_t rm = reg_no(dst);
    if (sz == 2)
        put8(0x66);
    uint8_t rex = 0;
    if (sz == 8)
        rex |= REX_W;
    if (rm >= 8)
        rex |= REX_B;
    bool byte_op = (sz == 1);
    bool force_rex = byte_op && (rm >= 4 && rm <= 7);
    if (rex || force_rex)
        put8(REX_BASE | rex);
    if (byte_op) {
        put8(OP_GRP1_IMM8B);
        put8((uint8_t)(0xc0u | ((uint32_t)digit << 3) | (rm & 7u)));
        put8((uint8_t)imm);
        return;
    }
    if (imm >= -128 && imm <= 127) {
        put8(OP_GRP1_IMM8);
        put8((uint8_t)(0xc0u | ((uint32_t)digit << 3) | (rm & 7u)));
        put8((uint8_t)(int8_t)imm);
        return;
    }
    put8(OP_GRP1_IMM32);
    put8((uint8_t)(0xc0u | ((uint32_t)digit << 3) | (rm & 7u)));
    put32((uint32_t)(i32)imm);
}

static void shift_ri(uint8_t digit, reg_t dst, i64 amount) {
    reg_size sz = rsz(dst);
    uint32_t rm = reg_no(dst);
    if (sz == 2)
        put8(0x66);
    uint8_t rex = 0;
    if (sz == 8)
        rex |= REX_W;
    if (rm >= 8)
        rex |= REX_B;
    if (rex)
        put8(REX_BASE | rex);
    put8(OP_GRP_SHIFT);
    put8((uint8_t)(0xc0u | ((uint32_t)digit << 3) | (rm & 7u)));
    put8((uint8_t)amount);
}

static void emit_neg(reg_t dst) {
    reg_size sz = rsz(dst);
    uint32_t rm = reg_no(dst);
    if (sz == 2)
        put8(0x66);
    uint8_t rex = 0;
    if (sz == 8)
        rex |= REX_W;
    if (rm >= 8)
        rex |= REX_B;
    if (rex)
        put8(REX_BASE | rex);
    put8(sz == 1 ? (OP_GRP3 & 0xfeu) : OP_GRP3);
    put8((uint8_t)(0xc0u | ((uint32_t)GRP3_NEG << 3) | (rm & 7u)));
}

void emit_fn_begin(emit_context_t *context) {
    body_len = 0;
    prol_len = 0;
    emit_to_prol = false;
    context->fn_named = false;
    active_ctx = context;
}

void emit_fn_end(emit_context_t *context) {
    uint32_t base = code_len;
    uint32_t delta = base + prol_len;
    if (code_len + prol_len + body_len > CODE_CAP) {
        report_error("x86_64-bin: code buffer overflow\n");
        return;
    }
    memcpy(code + code_len, prol, prol_len);
    code_len += prol_len;
    memcpy(code + code_len, body, body_len);
    code_len += body_len;

    for (uint32_t i = 0; i < nlabels; i++) {
        if (!labels[i].done) {
            labels[i].at += delta;
            labels[i].done = true;
        }
    }
    for (uint32_t i = 0; i < nfixups; i++) {
        if (!fixups[i].done) {
            fixups[i].site += delta;
            fixups[i].done = true;
        }
    }

    if (context->fn_named) {
        if (nfnsyms < LABEL_CAP) {
            snprintf(fn_syms[nfnsyms].key, KEY_CAP, "%.*s",
                     (int)str_len(context->fn_name), context->fn_name.data);
            fn_syms[nfnsyms].at = base;
            nfnsyms++;
        }
        if (str_eq(context->fn_name, STR("main")))
            entry_off = base;
    }

    body_len = 0;
    prol_len = 0;
    emit_to_prol = false;
}

static bool find_at(const label_t *tab, uint32_t n, const char *key, uint32_t *out) {
    for (uint32_t j = 0; j < n; j++) {
        if (strcmp(tab[j].key, key) == 0) {
            *out = tab[j].at;
            return true;
        }
    }
    return false;
}

void bin_emit(bin_image *image) {
    for (uint32_t i = 0; i < nfixups; i++) {
        const fixup_t *fx = &fixups[i];
        if (fx->kind == FX_STR) {
            int32_t rel = (int32_t)(code_len + fx->aux) - (int32_t)(fx->site + 4);
            memcpy(code + fx->site, &rel, 4);
            continue;
        }
        if (fx->kind == FX_CALL) {
            uint32_t target;
            if (find_at(fn_syms, nfnsyms, fx->key, &target)) {
                int32_t rel = (int32_t)target - (int32_t)(fx->site + 4);
                memcpy(code + fx->site, &rel, 4);
                continue;
            }
            uint32_t imp = n_imports;
            for (uint32_t k = 0; k < n_imports; k++) {
                if (strcmp(imports[k].name, fx->key) == 0) {
                    imp = k;
                    break;
                }
            }
            if (imp == n_imports) {
                if (n_imports >= IMPORT_CAP) {
                    report_error("x86_64-bin: import overflow\n");
                    continue;
                }
                char *name = malloc(KEY_CAP);
                if (!name)
                    malloc_failed();
                memcpy(name, fx->key, KEY_CAP);
                imports[n_imports++].name = name;
            }
            extcalls[n_extcalls].site = fx->site;
            extcalls[n_extcalls].import = imp;
            n_extcalls++;
            continue;
        }
        uint32_t target;
        if (!find_at(labels, nlabels, fx->key, &target)) {
            report_error("x86_64-bin: unresolved label '%s'\n", fx->key);
            continue;
        }
        int32_t rel = (int32_t)target - (int32_t)(fx->site + 4);
        memcpy(code + fx->site, &rel, 4);
    }

    static uint8_t image_buf[CODE_CAP + CSTR_CAP];
    memcpy(image_buf, code, code_len);
    memcpy(image_buf + code_len, cstrs, cstrs_len);
    image->text = image_buf;
    image->text_size = code_len + cstrs_len;
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
    if (value == 0) {
        reg_t d = dst;
        if (rsz(d) > 4)
            d.rsize = 4;
        encode_rr(OP_XOR_RM_R, rsz(d), reg_no(d), reg_no(d));
        return;
    }
    reg_size sz = rsz(dst);
    uint32_t r = reg_no(dst);
    if (sz <= 4) {
        if (sz == 2)
            put8(0x66);
        if (r >= 8)
            put8(REX_BASE | REX_B);
        if (sz == 1) {
            bool force_rex = (r >= 4 && r <= 7);
            if (force_rex && r < 8)
                put8(REX_BASE);
            put8((uint8_t)(OP_MOV_IMM_R8 + (r & 7u)));
            put8((uint8_t)value);
            return;
        }
        put8((uint8_t)(OP_MOV_IMM_R + (r & 7u)));
        if (sz == 2) {
            put8((uint8_t)value);
            put8((uint8_t)((u64)value >> 8));
        } else {
            put32((uint32_t)value);
        }
        return;
    }
    if (value >= INT32_MIN && value <= INT32_MAX) {
        put8(REX_BASE | REX_W | (r >= 8 ? REX_B : 0));
        put8(OP_MOV_IMM_RM);
        put8((uint8_t)(0xc0u | (r & 7u)));
        put32((uint32_t)(i32)value);
        return;
    }
    if (value > 0 && value <= (i64)UINT32_MAX) {
        if (r >= 8)
            put8(REX_BASE | REX_B);
        put8((uint8_t)(OP_MOV_IMM_R + (r & 7u)));
        put32((uint32_t)value);
        return;
    }
    put8(REX_BASE | REX_W | (r >= 8 ? REX_B : 0));
    put8((uint8_t)(OP_MOV_IMM_R + (r & 7u)));
    put64((uint64_t)value);
}

void emit_zero_out(reg_t dst) {
    emit_mov(dst, 0);
}

void emit_mov_reg(reg_t dst, reg_t src) {
    if (dst.rsize && src.rsize && dst.rsize > src.rsize) {
        if (src.dtype.base && src.dtype.base->sign) {
            if (dst.rsize == 8 && src.rsize == 4) {
                uint8_t rex = REX_W;
                if (reg_no(dst) >= 8)
                    rex |= REX_R;
                if (reg_no(src) >= 8)
                    rex |= REX_B;
                put8(REX_BASE | rex);
                put8(OP_MOVSXD);
                put8((uint8_t)(0xc0u | ((reg_no(dst) & 7u) << 3) | (reg_no(src) & 7u)));
                return;
            }
            reg_size ssz = src.rsize;
            reg_size dsz = dst.rsize;
            if (dsz == 2)
                put8(0x66);
            uint8_t rex = 0;
            if (dsz == 8)
                rex |= REX_W;
            if (reg_no(dst) >= 8)
                rex |= REX_R;
            if (reg_no(src) >= 8)
                rex |= REX_B;
            bool force_rex = (ssz == 1) && (reg_no(src) >= 4 && reg_no(src) <= 7);
            if (rex || force_rex)
                put8(REX_BASE | rex);
            put8(OP_0F);
            put8(ssz == 1 ? OP_MOVSX_8 : OP_MOVSX_16);
            put8((uint8_t)(0xc0u | ((reg_no(dst) & 7u) << 3) | (reg_no(src) & 7u)));
            return;
        }
        if (dst.rsize == 8 && src.rsize == 4) {
            reg_t d = dst;
            d.rsize = 4;
            encode_rr(OP_MOV_RM_R, 4, reg_no(src), reg_no(d));
            return;
        }
        reg_size ssz = src.rsize;
        reg_size dsz = dst.rsize;
        if (dsz == 2)
            put8(0x66);
        uint8_t rex = 0;
        if (dsz == 8)
            rex |= REX_W;
        if (reg_no(dst) >= 8)
            rex |= REX_R;
        if (reg_no(src) >= 8)
            rex |= REX_B;
        bool force_rex = (ssz == 1) && (reg_no(src) >= 4 && reg_no(src) <= 7);
        if (rex || force_rex)
            put8(REX_BASE | rex);
        put8(OP_0F);
        put8(ssz == 1 ? OP_MOVZX_8 : OP_MOVZX_16);
        put8((uint8_t)(0xc0u | ((reg_no(dst) & 7u) << 3) | (reg_no(src) & 7u)));
        return;
    }
    reg_t d = dst;
    if (d.rsize < src.rsize)
        d.rsize = src.rsize;
    encode_rr(OP_MOV_RM_R, rsz(d), reg_no(src), reg_no(d));
}

static const reg_t rax = {
    .rsize = sizeof(void *), .reg_type = SCRATCH, .offset = 0,
    .dtype = { .decl = {{ .tag = DK_ADDR, .amount = 1 }}, .decl_len = 1 },
};

void emit_add(reg_t dst, reg_t lhs, i64 rhs) {
    encode_mem(OP_LEA, rsz(dst), reg_no(dst), reg_no(lhs), false, 0, 1, (int32_t)rhs);
}

void emit_add_reg(reg_t dst, reg_t lhs, reg_t rhs) {
    if (lhs.rsize != rhs.rsize) {
        reg_t wide = lhs.rsize > rhs.rsize ? lhs : rhs;
        const reg_t narrow = lhs.rsize > rhs.rsize ? rhs : lhs;
        wide.rsize = dst.rsize;
        emit_mov_reg(dst, narrow);
        alu_rr(OP_ADD_RM_R, dst, wide);
        return;
    }
    encode_mem(OP_LEA, rsz(dst), reg_no(dst), reg_no(lhs), true, reg_no(rhs), 1, 0);
}

void emit_sub(reg_t dst, reg_t lhs, i64 rhs) {
    encode_mem(OP_LEA, rsz(dst), reg_no(dst), reg_no(lhs), false, 0, 1, (int32_t)(-rhs));
}

void emit_sub_reg(reg_t dst, reg_t lhs, reg_t rhs) {
    if (lhs.rsize == rhs.rsize) {
        encode_rr(OP_MOV_RM_R, rsz(dst), reg_no(lhs), reg_no(dst));
        alu_rr(OP_SUB_RM_R, dst, rhs);
        return;
    }
    reg_t wlhs = lhs;
    reg_t wrhs = rhs;
    wlhs.rsize = dst.rsize;
    wrhs.rsize = dst.rsize;
    if (rhs.rsize < lhs.rsize) {
        emit_mov_reg(dst, rhs);
        alu_rr(OP_SUB_RM_R, dst, wlhs);
        emit_neg(dst);
        return;
    }
    emit_mov_reg(dst, lhs);
    alu_rr(OP_SUB_RM_R, dst, wrhs);
}

void emit_cmp(reg_t lhs, i64 rhs) {
    alu_ri(ALU_CMP, lhs, rhs);
}

void emit_cmp_reg(reg_t lhs, reg_t rhs, cond_t cond) {
    if (lhs.rsize == rhs.rsize) {
        alu_rr(OP_CMP_RM_R, lhs, rhs);
        return;
    }
    reg_t tmp = { .reg_type = SCRATCH, .offset = 0 };
    tmp.rsize = lhs.rsize > rhs.rsize ? lhs.rsize : rhs.rsize;
    if (rhs.rsize < lhs.rsize) {
        reg_t wlhs = lhs;
        wlhs.rsize = tmp.rsize;
        emit_mov_reg(tmp, rhs);
        alu_rr(OP_CMP_RM_R, wlhs, tmp);
        return;
    }
    reg_t wlhs = lhs;
    reg_t wrhs = rhs;
    wlhs.rsize = tmp.rsize;
    wrhs.rsize = tmp.rsize;
    if (cond == COND_EQ || cond == COND_NE) {
        alu_rr(OP_CMP_RM_R, wlhs, wrhs);
        return;
    }
    emit_mov_reg(tmp, lhs);
    alu_rr(OP_CMP_RM_R, tmp, wrhs);
}

void emit_string_lit(reg_t dst, const str *s) {
    const char *p = s->data;
    size_t n = str_len(*s);
    if (n >= 2 && p[0] == '"') {
        p += 1;
        n -= 2;
    }
    if (cstrs_len + n + 1 > CSTR_CAP) {
        report_error("x86_64-bin: cstring buffer overflow\n");
        return;
    }
    uint32_t str_off = cstrs_len;
    memcpy(cstrs + cstrs_len, p, n);
    cstrs_len += (uint32_t)n;
    cstrs[cstrs_len++] = '\0';

    reg_size sz = rsz(dst);
    uint32_t r = reg_no(dst);
    uint8_t rex = 0;
    if (sz == 8)
        rex |= REX_W;
    if (r >= 8)
        rex |= REX_R;
    if (rex)
        put8(REX_BASE | rex);
    put8(OP_LEA);
    put8((uint8_t)(0x00u | ((r & 7u) << 3) | 5u));

    if (nfixups >= FIXUP_CAP) {
        report_error("x86_64-bin: fixup overflow\n");
        return;
    }
    fixup_t *fx = &fixups[nfixups++];
    fx->site = cur_off();
    fx->aux = str_off;
    fx->kind = FX_STR;
    put32(0);
}

void emit_lsl(reg_t dst, reg_t lhs, i64 rhs) {
    if (rhs >= 4) {
        encode_rr(OP_MOV_RM_R, rsz(dst), reg_no(lhs), reg_no(dst));
        shift_ri(SH_SHL, dst, rhs);
    } else if (rhs == 0) {
        emit_mov_reg(dst, lhs);
    } else if (rhs == 1) {
        emit_add_reg(dst, lhs, lhs);
    } else {
        encode_mem(OP_LEA, rsz(dst), reg_no(dst), reg_no(lhs), true, reg_no(lhs),
                   (uint32_t)(1 << rhs), 0);
    }
}

void emit_str_reg(reg_t dst, reg_t src, int offset) {
    encode_mem(OP_MOV_RM_R, rsz(src), reg_no(src), reg_no(dst), false, 0, 1, offset);
}

static void emit_mem_imm(reg_size sz, reg_t dst, int offset, i64 value) {
    uint32_t base = reg_no(dst);
    if (sz == 2)
        put8(0x66);
    uint8_t rex = 0;
    if (sz == 8)
        rex |= REX_W;
    if (base >= 8)
        rex |= REX_B;
    if (rex)
        put8(REX_BASE | rex);
    put8(sz == 1 ? (OP_MOV_IMM_RM & 0xfeu) : OP_MOV_IMM_RM);

    uint32_t base_low3 = base & 7u;
    uint8_t mod = choose_mod(offset, base_low3);
    bool need_sib = base_low3 == 4u;
    if (need_sib) {
        put8((uint8_t)(((uint32_t)mod << 6) | (0u << 3) | 4u));
        put8((uint8_t)((0u << 6) | (4u << 3) | base_low3));
    } else {
        put8((uint8_t)(((uint32_t)mod << 6) | (0u << 3) | base_low3));
    }
    put_disp(offset, mod);

    if (sz == 1)
        put8((uint8_t)value);
    else if (sz == 2)
        put8((uint8_t)value), put8((uint8_t)((u64)value >> 8));
    else
        put32((uint32_t)(i32)value);
}

void emit_str_imm(reg_t dst, i64 value, int offset) {
    reg_size sz = (reg_size)dst.dtype.base->size;
    if (sz >= 8 && (value < INT32_MIN || value > INT32_MAX)) {
        emit_mem_imm(4, dst, offset, (i64)(i32)(u32)value);
        emit_mem_imm(4, dst, offset + 4, (i64)(i32)(u32)((u64)value >> 32));
        return;
    }
    emit_mem_imm(sz, dst, offset, value);
}

void emit_ldr(reg_t dst, reg_t src, int offset) {
    encode_mem(OP_MOV_R_RM, rsz(dst), reg_no(dst), reg_no(src), false, 0, 1, offset);
}

void emit_str_regoff(reg_t dst, reg_t src, reg_t offset) {
    encode_mem(OP_LEA, 8, RAX, reg_no(dst), true, reg_no(offset), 1, 0);
    emit_str_reg(rax, src, 0);
}

void emit_str_imm_regoff(reg_t dst, i64 value, reg_t offset) {
    encode_mem(OP_LEA, 8, RAX, reg_no(dst), true, reg_no(offset), 1, 0);
    reg_t addr = rax;
    addr.dtype.base = dst.dtype.base;
    emit_str_imm(addr, value, 0);
}

void emit_ldr_reg(reg_t dst, reg_t src, reg_t offset) {
    encode_mem(OP_LEA, rsz(dst), reg_no(dst), reg_no(src), true, reg_no(offset), 1, 0);
    emit_ldr(dst, dst, 0);
}

/* --- control flow --- */

void emit_branch(str fn_name, str label, int index) {
    if (nfixups >= FIXUP_CAP) {
        report_error("x86_64-bin: fixup overflow\n");
        return;
    }
    put8(OP_JMP_REL32);
    fixup_t *fx = &fixups[nfixups++];
    make_key(fx->key, fn_name, label, index);
    fx->site = cur_off();
    fx->kind = FX_JMP;
    put32(0);
}

bool emit_branch_cond(cond_t condition, str fn_name, str label, int index) {
    if (condition >= (cond_t)(sizeof cond_cc)) {
        report_error("x86_64-bin: unknown condition %d\n", condition);
        return false;
    }
    if (nfixups >= FIXUP_CAP) {
        report_error("x86_64-bin: fixup overflow\n");
        return false;
    }
    put8(OP_0F);
    put8((uint8_t)(OP_JCC_REL32 + cond_cc[condition]));
    fixup_t *fx = &fixups[nfixups++];
    make_key(fx->key, fn_name, label, index);
    fx->site = cur_off();
    fx->kind = FX_JCC;
    put32(0);
    return true;
}

void emit_label(str fn_name, str label, int index) {
    if (nlabels >= LABEL_CAP) {
        report_error("x86_64-bin: label overflow\n");
        return;
    }
    label_t *l = &labels[nlabels++];
    make_key(l->key, fn_name, label, index);
    l->at = cur_off();
}

void emit_cond_set(reg_t dst, cond_t cond) {
    if (cond >= (cond_t)(sizeof cond_cc)) {
        report_error("x86_64-bin: unknown condition %d\n", cond);
        return;
    }
    uint32_t r = reg_no(dst);
    bool force_rex = (r >= 4 && r <= 7);
    uint8_t rex = 0;
    if (r >= 8)
        rex |= REX_B;
    if (rex || force_rex)
        put8(REX_BASE | rex);
    put8(OP_0F);
    put8((uint8_t)(OP_SETCC + cond_cc[cond]));
    put8((uint8_t)(0xc0u | (r & 7u)));

    reg_t wide = dst;
    if (wide.rsize <= 1)
        wide.rsize = 4;
    reg_t byted = dst;
    byted.rsize = 1;
    byted.dtype.base = NULL;
    byted.dtype.decl_len = 0;
    emit_mov_reg(wide, byted);
}

/* --- function management --- */

void emit_fn_prologue_epilogue(const parser_context *pc) {
    size_t stack_size = 0;
    size_t shadow_size = 0;
    bool calls_fn = pc->calls_fn;
    if (calls_fn) {
        shadow_size = 32;
        stack_size += shadow_size;
    }
    size_t locals_size = (size_t)pc->stack_size;
    stack_size += locals_size;
    stack_size = ALIGN_TO(stack_size, (size_t)0x10);

    int regs_to_save = pc->nreg_count;
    int tmp = regs_to_save + (locals_size ? 1 : 0) + (calls_fn ? 1 : 0);
    if (tmp % 2 == 1)
        stack_size += 8;

    emit_to_prol = true;
    if (locals_size)
        put8((uint8_t)(OP_PUSH_R + RBP));
    for (int i = 0; i < regs_to_save; ++i) {
        uint32_t r = callee_regs[i];
        if (r >= 8)
            put8(REX_BASE | REX_B);
        put8((uint8_t)(OP_PUSH_R + (r & 7u)));
    }
    if (locals_size) {
        if (shadow_size)
            encode_mem(OP_LEA, 8, RBP, RSP, false, 0, 1, -(int32_t)shadow_size);
        else
            encode_rr(OP_MOV_RM_R, 8, RSP, RBP);
    }
    if (stack_size) {
        reg_t rsp_reg = { .reg_type = STACK, .rsize = 8 };
        alu_ri(ALU_SUB, rsp_reg, (i64)stack_size);
    }
    emit_to_prol = false;

    if (stack_size) {
        reg_t rsp_reg = { .reg_type = STACK, .rsize = 8 };
        alu_ri(ALU_ADD, rsp_reg, (i64)stack_size);
    }
    for (int i = regs_to_save - 1; i >= 0; --i) {
        uint32_t r = callee_regs[i];
        if (r >= 8)
            put8(REX_BASE | REX_B);
        put8((uint8_t)(OP_POP_R + (r & 7u)));
    }
    if (locals_size)
        put8((uint8_t)(OP_POP_R + RBP));
}

void emit_fn_call(const str *s) {
    if (nfixups >= FIXUP_CAP) {
        report_error("x86_64-bin: fixup overflow\n");
        return;
    }
    put8(OP_CALL_REL32);
    fixup_t *fx = &fixups[nfixups++];
    snprintf(fx->key, KEY_CAP, "%.*s", (int)str_len(*s), s->data);
    fx->site = cur_off();
    fx->kind = FX_CALL;
    put32(0);
}

void emit_fn(str fn_name) {
    if (active_ctx) {
        active_ctx->fn_name = fn_name;
        active_ctx->fn_named = true;
    }
}

void emit_ret(void) {
    put8(OP_RET);
}

/* --- aggregates --- */

bool emit_eightbyte_struct(reg_t dst, const dtype_t *dtype, const dyn_agg_member *args,
                           int *index, size_t *size_out, size_t limit) {
    (void)limit;
    type_t *type = dtype->base;
    ptrdiff_t member_count = args->cur - args->begin;

    bool is_arr = !dtype_empty(dtype) && dtype_top(dtype).tag == DK_ARRAY;

    if (is_arr) {
        size_t remaining = (size_t)(member_count - *index) * dtype->base->size;
        size_t span = remaining > 8 ? 8 : remaining;
        dst.rsize = span > 4 ? 8 : (span > 2 ? 4 : (span > 1 ? 2 : 1));
    }

    reg_t shift_tmp = { .reg_type = SCRATCH, .offset = 1, .rsize = 8 };
    if (dst.reg_type == SCRATCH && dst.offset >= shift_tmp.offset)
        shift_tmp.offset = dst.offset + 1;

    bool cleared = false;
    size_t size_acc = 0;

    for (ptrdiff_t i = *index; i < member_count; ++i, *index = (int)i) {
        member_t local_memb;
        member_t *memb;
        if (is_arr) {
            local_memb = (member_t){ .dtype = *dtype, .offset = (size_t)i * dtype->base->size };
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
                if (shifted == (i32)shifted) {
                    alu_ri(ALU_OR, dst, shifted);
                } else {
                    emit_mov(shift_tmp, shifted);
                    alu_rr(OP_OR_RM_R, dst, shift_tmp);
                }
            }
        } else if (arg->tag == REG) {
            reg_t reg = arg->reg;
            if (reg.reg_type == STACK) {
                const reg_t frame = { .reg_type = FRAME, .rsize = 8 };
                emit_sub(shift_tmp, frame, (i64)reg.offset);
                reg = shift_tmp;
            }
            reg.rsize = memb_size < 4 ? (reg_size)memb_size : (memb_size <= 4 ? 4 : 8);

            if (local_offset_bits == 0 && reg.rsize == dst.rsize) {
                if (!cleared)
                    alu_rr(OP_MOV_RM_R, dst, reg);
                else
                    alu_rr(OP_OR_RM_R, dst, reg);
            } else {
                if (reg.rsize < 8)
                    emit_mov_reg(shift_tmp, reg);
                else
                    alu_rr(OP_MOV_RM_R, shift_tmp, reg);
                if (local_offset_bits != 0)
                    shift_ri(SH_SHL, shift_tmp, (i64)local_offset_bits);
                if (!cleared)
                    alu_rr(OP_MOV_RM_R, dst, shift_tmp);
                else
                    alu_rr(OP_OR_RM_R, dst, shift_tmp);
            }
            cleared = true;
        }
    }

    *size_out += size_acc;
    return cleared;
}

void emit_store_eightbytes(reg_t base, i64 offset, reg_t lo, bool lo_written,
                           reg_t hi, bool hi_written, bool has_hi) {
    if (has_hi) {
        lo.rsize = 8;
        hi.rsize = 8;
    }
    if (!lo_written)
        encode_rr(OP_XOR_RM_R, rsz(lo), reg_no(lo), reg_no(lo));
    emit_str_reg(base, lo, (int)offset);
    if (has_hi) {
        if (!hi_written)
            encode_rr(OP_XOR_RM_R, rsz(hi), reg_no(hi), reg_no(hi));
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
            shift_ri(SH_SHR, full, (i64)chunk * 8);
        }
    }
}

void emit_zerofill(reg_t base, i64 offset, const dtype_t *dtype) {
    size_t size = dtype_size(dtype);
    reg_t zero = { .reg_type = SCRATCH, .offset = 1, .rsize = 8, .dtype = { .base = dtype->base } };
    encode_rr(OP_XOR_RM_R, 4, reg_no(zero), reg_no(zero));

    reg_size rsize = 8;
    while (size > 0) {
        while (size >= rsize) {
            zero.rsize = rsize;
            emit_str_reg(base, zero, (int)offset);
            size -= rsize;
            offset += rsize;
        }
        if (rsize == 1)
            break;
        rsize /= 2;
    }
}

void emit_make_array(reg_t dst, type_t *type, u32 len, dyn_regable *args) {}
void emit_store_array(reg_t dst, i64 offset, type_t *type, u32 len, dyn_regable *args) {}

void emit_array_access(reg_t dst, reg_t src, reg_t offset, load_store_t is_store) {
    dtype_t *dtype = &src.dtype;
    size_t elem_size = dtype->base->size;

    if (src.reg_type == STACK && src.offset) {
        reg_t tmp_src = src;
        tmp_src.reg_type = SCRATCH;
        tmp_src.offset = 2;
        tmp_src.rsize = 8;
        const reg_t frame = { .reg_type = FRAME, .rsize = 8 };
        emit_sub(tmp_src, frame, src.offset);
        src = tmp_src;
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
            if (is_store)
                encode_mem(OP_MOV_RM_R, rsz(dst), reg_no(dst), reg_no(src), true,
                           reg_no(offset), (uint32_t)elem_size, 0);
            else
                encode_mem(OP_MOV_R_RM, rsz(dst), reg_no(dst), reg_no(src), true,
                           reg_no(offset), (uint32_t)elem_size, 0);
            return;
        }
    }

    reg_t idx = { .reg_type = SCRATCH, .offset = 1, .rsize = 8 };
    emit_mov_reg(idx, offset);
    if (elem_size <= 127) {
        put8(REX_BASE | REX_W | (reg_no(idx) >= 8 ? (REX_R | REX_B) : 0));
        put8(OP_IMUL_IMM8);
        put8((uint8_t)(0xc0u | ((reg_no(idx) & 7u) << 3) | (reg_no(idx) & 7u)));
        put8((uint8_t)elem_size);
    } else {
        put8(REX_BASE | REX_W | (reg_no(idx) >= 8 ? (REX_R | REX_B) : 0));
        put8(OP_IMUL_IMM32);
        put8((uint8_t)(0xc0u | ((reg_no(idx) & 7u) << 3) | (reg_no(idx) & 7u)));
        put32((uint32_t)elem_size);
    }
    emit_add_reg(idx, src, idx);

    if (is_store)
        emit_str_reg(idx, dst, 0);
    else
        emit_ldr(dst, idx, 0);
}

void emit_elem_addr(reg_t dst, reg_t object, reg_t index) {
    reg_t base = { .reg_type = SCRATCH, .offset = 0, .rsize = sizeof(void *) };
    if (object.reg_type == STACK) {
        const reg_t frame = { .reg_type = FRAME, .rsize = sizeof(void *) };
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
        encode_mem(OP_LEA, rsz(dst), reg_no(dst), reg_no(base), true, reg_no(index),
                   (uint32_t)elem_size, 0);
    } else {
        reg_t idx = { .reg_type = SCRATCH, .offset = 1, .rsize = 8 };
        emit_mov_reg(idx, index);
        if (elem_size <= 127) {
            put8(REX_BASE | REX_W | (reg_no(idx) >= 8 ? (REX_R | REX_B) : 0));
            put8(OP_IMUL_IMM8);
            put8((uint8_t)(0xc0u | ((reg_no(idx) & 7u) << 3) | (reg_no(idx) & 7u)));
            put8((uint8_t)elem_size);
        } else {
            put8(REX_BASE | REX_W | (reg_no(idx) >= 8 ? (REX_R | REX_B) : 0));
            put8(OP_IMUL_IMM32);
            put8((uint8_t)(0xc0u | ((reg_no(idx) & 7u) << 3) | (reg_no(idx) & 7u)));
            put32((uint32_t)elem_size);
        }
        emit_add_reg(dst, base, idx);
    }
}

#pragma clang diagnostic pop

const size_t default_register_size = 8;
