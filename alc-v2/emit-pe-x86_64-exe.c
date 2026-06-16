#include <stdint.h>
#include <string.h>
#include <windows.h>

const uint8_t scratch_regs[] = { 0, 1, 2, 8, 9, 10, 11 };
const uint8_t callee_regs[]  = { 3, 6, 7, 12, 13, 14, 15 };
const uint8_t param_regs[]   = { 1, 2, 8, 9, 10, 11 };
const uint8_t ret_regs[]     = { 0, 2 };

static const uint16_t pe_machine = IMAGE_FILE_MACHINE_AMD64;

static const uint32_t pe_entry_stub_size = 16;
static const uint32_t pe_thunk_size      = 6;

static void pe_build_entry_stub(uint8_t *p, uint32_t stub_rva, uint32_t main_rva,
                                uint32_t exit_thunk_rva) {
    int32_t rel_main = (int32_t)(main_rva - (stub_rva + 9));
    int32_t rel_exit = (int32_t)(exit_thunk_rva - (stub_rva + 16));
    p[0] = 0x48; p[1] = 0x83; p[2] = 0xec; p[3] = 0x28;
    p[4] = 0xe8; memcpy(p + 5, &rel_main, 4);
    p[9] = 0x89; p[10] = 0xc1;
    p[11] = 0xe8; memcpy(p + 12, &rel_exit, 4);
}

static void pe_build_thunk(uint8_t *p, uint32_t thunk_rva, uint32_t iat_slot_rva) {
    int32_t disp = (int32_t)(iat_slot_rva - (thunk_rva + 6));
    p[0] = 0xff; p[1] = 0x25; memcpy(p + 2, &disp, 4);
}

static void pe_patch_call(uint8_t *site, uint32_t site_rva, uint32_t target_rva) {
    int32_t rel = (int32_t)(target_rva - (site_rva + 4));
    memcpy(site, &rel, 4);
}

#include "emit_helper-pe-exe.h"
