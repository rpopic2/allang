#define _DEFAULT_SOURCE
#include <stdint.h>
#include <string.h>
#include <elf.h>

static const uint16_t elf_machine         = EM_X86_64;
static const uint32_t elf_reloc_jump_slot = R_X86_64_JUMP_SLOT;
static const char     elf_interp[]        = "/lib64/ld-linux-x86-64.so.2";

static const uint32_t elf_start_stub_size     = 14;
static const uint32_t elf_dyn_start_stub_size = 12;
static const uint32_t elf_plt0_size           = 16;
static const uint32_t elf_plt_entry_size      = 16;

/* static _start: call main; mov edi, eax; mov eax, 60; syscall */
static void elf_build_start_stub(uint8_t *p, uint64_t stub_vaddr, uint64_t main_vaddr) {
    int32_t rel = (int32_t)((int64_t)main_vaddr - (int64_t)(stub_vaddr + 5));
    p[0] = 0xe8; memcpy(p + 1, &rel, 4);
    p[5] = 0x89; p[6] = 0xc7;
    p[7] = 0xb8; p[8] = 60; p[9] = 0; p[10] = 0; p[11] = 0;
    p[12] = 0x0f; p[13] = 0x05;
}

/* dynamic _start: call main; mov edi, eax; call exit@plt */
static void elf_build_dyn_start_stub(uint8_t *p, uint64_t stub_vaddr,
                                     uint64_t main_vaddr, uint64_t exit_plt_vaddr) {
    int32_t rel_main = (int32_t)((int64_t)main_vaddr     - (int64_t)(stub_vaddr + 5));
    int32_t rel_exit = (int32_t)((int64_t)exit_plt_vaddr - (int64_t)(stub_vaddr + 12));
    p[0] = 0xe8; memcpy(p + 1, &rel_main, 4);
    p[5] = 0x89; p[6] = 0xc7;
    p[7] = 0xe8; memcpy(p + 8, &rel_exit, 4);
}

/* PLT[0]: push [GOT[1]]; jmp [GOT[2]]; nop4 */
static void elf_build_plt0(uint8_t *p, uint64_t plt0_vaddr, uint64_t got_vaddr) {
    int32_t r1 = (int32_t)((int64_t)(got_vaddr + 8)  - (int64_t)(plt0_vaddr + 6));
    int32_t r2 = (int32_t)((int64_t)(got_vaddr + 16) - (int64_t)(plt0_vaddr + 12));
    p[0] = 0xff; p[1] = 0x35; memcpy(p + 2, &r1, 4);
    p[6] = 0xff; p[7] = 0x25; memcpy(p + 8, &r2, 4);
    p[12] = 0x0f; p[13] = 0x1f; p[14] = 0x40; p[15] = 0x00;
}

/* PLT[n]: jmp [GOT[n]]; push index; jmp PLT[0].
   Returns the initial GOT value (points at the push, for lazy resolution). */
static uint64_t elf_build_plt_entry(uint8_t *p, uint32_t index, uint64_t plt_vaddr,
                                    uint64_t plt0_vaddr, uint64_t got_entry_vaddr) {
    int32_t jmp_got  = (int32_t)((int64_t)got_entry_vaddr - (int64_t)(plt_vaddr + 6));
    int32_t jmp_plt0 = (int32_t)((int64_t)plt0_vaddr      - (int64_t)(plt_vaddr + 16));
    p[0] = 0xff; p[1] = 0x25; memcpy(p + 2, &jmp_got, 4);
    p[6] = 0x68; memcpy(p + 7, &index, 4);
    p[11] = 0xe9; memcpy(p + 12, &jmp_plt0, 4);
    return plt_vaddr + 6;
}

/* Patch a call rel32 site (site points at the 4-byte displacement field). */
static void elf_patch_call(uint8_t *site, uint64_t site_vaddr, uint64_t target_vaddr) {
    int32_t rel = (int32_t)((int64_t)target_vaddr - (int64_t)(site_vaddr + 4));
    memcpy(site, &rel, 4);
}

#include "emit_helper-elf-exe.h"
