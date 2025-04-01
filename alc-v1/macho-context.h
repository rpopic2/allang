#pragma once

#include "list.h"
#include "slice.h"
#include "strgen.h"
#include <mach-o/arm64/reloc.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>


ls_char strtab;
int strtab_idx;

typedef struct nlist_64 stabe;
ls (stabe)
ls_stabe stab_loc;
ls_stabe stab_ext;
ls_stabe stab_und;

typedef struct relocation_info relocent;
ls (relocent)
ls_relocent relocents;

ls_int to_push;

void macho_context_init() {
    ls_new_relocent(&relocents, 128, "relocation entries");

    ls_new_stabe(&stab_loc, 128, "local symbol table");
    ls_new_stabe(&stab_ext, 128, "external symbol table");
    ls_new_stabe(&stab_und, 128, "undefined symbol table");

    ls_new_char(&strtab, 1024, "strtab");
    ls_add_char(&strtab, '\0');

    ls_new_int(&to_push, 128, "symbol table entries to push n_value");
}

void macho_stab_ext(str entry) {
    const struct nlist_64 entry_main = {
        .n_un.n_strx = strtab.count,
        .n_type = N_EXT | N_TYPE,
        .n_sect = 0x1,
        .n_desc = 0x0,
        .n_value = 0L,
    };
    ls_add_stabe(&stab_ext, entry_main);

    ls_addran_char(&strtab, entry.data, entry.len);
    ls_add_char(&strtab, '\0');
}

void macho_stab_undef(str s) {
    const struct nlist_64 entry = {
        .n_un.n_strx = strtab.count,
        .n_type = N_EXT,
        .n_sect = NO_SECT,
        .n_desc = 0x0,
        .n_value = 0L,
    };
    ls_add_stabe(&stab_und, entry);

    ls_addran_char(&strtab, s.data, s.len);
    ls_add_char(&strtab, '\0');
}

void macho_stab_stringlit() {
    int stab_idx = stab_loc.count;

    const struct nlist_64 entry = {
        .n_un.n_strx = strtab.count,
        .n_type = N_TYPE,
        .n_sect = 1,
        .n_desc = 0x0,
        .n_value = 0,
    };
    ls_add_stabe(&stab_loc, entry);
    ls_add_int(&to_push, stab_idx);

    const char * name =  "__str";
    ls_addran_char(&strtab, name, strlen(name));
    str index = strgen_next();
    ls_addran_char(&strtab, index.data, index.len + 1);
}

void macho_relocent(fat stackcode, enum reloc_type_arm64 type, bool pcrel) {
    int stab_idx = stab_loc.count - 1;

    const struct relocation_info rel = {
        .r_address = (stackcode.end - stackcode.start) * sizeof (u32),
        .r_symbolnum = stab_idx,

        .r_pcrel = pcrel,
        .r_length = 2,
        .r_extern = true,
        .r_type = type,
    };
    ls_add_relocent(&relocents, rel);
}
