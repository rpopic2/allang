#pragma once

#include <stdint.h>

/* Architecture-specific pieces of the ELF executable. The OS-neutral ELF writer
   (emit-elf-exe.c) splices these into the _start stub, the PLT and the call
   sites; the linked bin backend (emit-x86_64-bin.c or emit-aarch64-bin.c)
   supplies them. All vaddr arguments are absolute virtual addresses. */

extern const uint16_t elf_machine;          /* e_machine */
extern const uint32_t elf_reloc_jump_slot;  /* R_*_JUMP_SLOT relocation type */
extern const char *elf_interp;              /* dynamic linker path */

extern const uint32_t elf_start_stub_size;      /* static _start: call main; exit(retval) */
extern const uint32_t elf_dyn_start_stub_size;  /* dynamic _start: call main; exit@plt */
extern const uint32_t elf_plt0_size;            /* PLT[0] resolver trampoline */
extern const uint32_t elf_plt_entry_size;       /* each PLT[n] */

void elf_build_start_stub(uint8_t *p, uint64_t stub_vaddr, uint64_t main_vaddr);
void elf_build_dyn_start_stub(uint8_t *p, uint64_t stub_vaddr,
                              uint64_t main_vaddr, uint64_t exit_plt_vaddr);
void elf_build_plt0(uint8_t *p, uint64_t plt0_vaddr, uint64_t got_vaddr);

/* Returns the value the GOT entry is initialized with for lazy binding. */
uint64_t elf_build_plt_entry(uint8_t *p, uint32_t index, uint64_t plt_vaddr,
                             uint64_t plt0_vaddr, uint64_t got_entry_vaddr);

void elf_patch_call(uint8_t *site, uint64_t site_vaddr, uint64_t target_vaddr);
