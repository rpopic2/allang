#define _DEFAULT_SOURCE
#include <stdint.h>
#include <string.h>
#include <elf.h>

static const uint16_t elf_machine         = EM_AARCH64;
static const uint32_t elf_reloc_jump_slot = R_AARCH64_JUMP_SLOT;
static const char     elf_interp[]        = "/lib/ld-linux-aarch64.so.1";

static const uint32_t elf_start_stub_size     = 12;
static const uint32_t elf_dyn_start_stub_size = 8;
static const uint32_t elf_plt0_size           = 32;
static const uint32_t elf_plt_entry_size      = 16;

static void put_le32(uint8_t *p, uint32_t w) { memcpy(p, &w, 4); }

static uint32_t enc_bl(uint64_t pc, uint64_t target) {
    int64_t off = (int64_t)target - (int64_t)pc;
    return 0x94000000u | ((uint32_t)(off >> 2) & 0x03FFFFFFu);
}

static uint32_t enc_adrp(uint32_t rd, uint64_t pc, uint64_t target) {
    int64_t delta = (int64_t)(target & ~0xFFFULL) - (int64_t)(pc & ~0xFFFULL);
    uint32_t imm = (uint32_t)((uint64_t)(delta >> 12) & 0x1FFFFFu);
    return 0x90000000u | ((imm & 3u) << 29) | (((imm >> 2) & 0x7FFFFu) << 5) | rd;
}

static uint32_t enc_ldr64(uint32_t rt, uint32_t rn, uint64_t target) {
    return 0xF9400000u | (((uint32_t)(target & 0xFFFu) >> 3) << 10) | (rn << 5) | rt;
}

static uint32_t enc_add64(uint32_t rd, uint32_t rn, uint64_t target) {
    return 0x91000000u | (((uint32_t)(target & 0xFFFu)) << 10) | (rn << 5) | rd;
}

static void elf_build_start_stub(uint8_t *p, uint64_t stub_vaddr, uint64_t main_vaddr) {
    put_le32(p + 0, enc_bl(stub_vaddr, main_vaddr));
    put_le32(p + 4, 0x52800BA8u);
    put_le32(p + 8, 0xD4000001u);
}

static void elf_build_dyn_start_stub(uint8_t *p, uint64_t stub_vaddr,
                                     uint64_t main_vaddr, uint64_t exit_plt_vaddr) {
    put_le32(p + 0, enc_bl(stub_vaddr,     main_vaddr));
    put_le32(p + 4, enc_bl(stub_vaddr + 4, exit_plt_vaddr));
}

static void elf_build_plt0(uint8_t *p, uint64_t plt0_vaddr, uint64_t got_vaddr) {
    uint64_t g2 = got_vaddr + 16;
    put_le32(p + 0,  0xA9BF7BF0u);
    put_le32(p + 4,  enc_adrp(16, plt0_vaddr + 4, g2));
    put_le32(p + 8,  enc_ldr64(17, 16, g2));
    put_le32(p + 12, enc_add64(16, 16, g2));
    put_le32(p + 16, 0xD61F0220u);
    put_le32(p + 20, 0xD503201Fu);
    put_le32(p + 24, 0xD503201Fu);
    put_le32(p + 28, 0xD503201Fu);
}

static uint64_t elf_build_plt_entry(uint8_t *p, uint32_t index, uint64_t plt_vaddr,
                                    uint64_t plt0_vaddr, uint64_t got_entry_vaddr) {
    (void)index;
    put_le32(p + 0,  enc_adrp(16, plt_vaddr, got_entry_vaddr));
    put_le32(p + 4,  enc_ldr64(17, 16, got_entry_vaddr));
    put_le32(p + 8,  enc_add64(16, 16, got_entry_vaddr));
    put_le32(p + 12, 0xD61F0220u);
    return plt0_vaddr;
}

static void elf_patch_call(uint8_t *site, uint64_t site_vaddr, uint64_t target_vaddr) {
    uint32_t w;
    memcpy(&w, site, 4);
    w = (w & ~0x03FFFFFFu) |
        ((uint32_t)(((int64_t)target_vaddr - (int64_t)site_vaddr) >> 2) & 0x03FFFFFFu);
    memcpy(site, &w, 4);
}

#include "emit_helper-elf-exe.h"
