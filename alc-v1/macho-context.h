#pragma once

#include "list.h"
#include "parser_util.h"
#include "stack_context.h"
#include "strgen.h"
#include <mach-o/arm64/reloc.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>


ls_char strtab;
int strtab_idx;

ls_char strings;

typedef struct nlist_64 stabe;
ls (stabe)
ls_stabe stab_loc;
ls_stabe stab_ext;
ls_stabe stab_und;

typedef struct relocation_info relocent;
ls (relocent)
ls_relocent relocents;

ls_int to_push;
ls_int to_push_ext;
ls_int to_push_und;

void macho_context_init() {
    ls_new_relocent(&relocents, 128, "relocation entries");

    ls_new_stabe(&stab_loc, 128, "local symbol table");
    ls_new_stabe(&stab_ext, 128, "external symbol table");
    ls_new_stabe(&stab_und, 128, "undefined symbol table");

    ls_new_char(&strtab, 1024, "strtab");
    ls_add_char(&strtab, '\0');

    ls_new_int(&to_push, 128, "symbol table entries to push n_value");
    ls_new_int(&to_push_und, 128, "relocation table entries, that needs r_symbolnum pushed by number of loc and ext");
    ls_new_int(&to_push_ext, 128, "relocation table entries, that needs r_symbolnum pushed by number of locr");
}

void macho_stab_ext(fat code, str s) {
    int len = fat_len(code) * sizeof (u32);
    const struct nlist_64 entry = {
        .n_un.n_strx = strtab.count,
        .n_type = N_EXT | N_TYPE,
        .n_sect = 0x1,
        .n_desc = 0x0,
        .n_value = len, // TODO
    };
    ls_add_stabe(&stab_ext, entry);
    printf("add stab entry, len %d, name ", stab_ext.count), strprint(s);

    ls_addran_char(&strtab, s.data, s.len);
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

void macho_stab_loc(int len, str s) {
    // int stab_idx = stab_loc.count;
    int len2 = fat_len((fat){_objcode, objcode}) * sizeof (u32);

    const struct nlist_64 entry = {
        .n_un.n_strx = strtab.count,
        .n_type = N_TYPE,
        .n_sect = 1,
        .n_desc = 0x0,
        .n_value = len2 + len,
    };
    ls_add_stabe(&stab_loc, entry);
    // ls_add_int(&to_push, stab_idx);

    printf("add stab_loc entry, len %d, name ", len), strprint(s);
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
        .n_value = strings.count,
    };
    ls_add_stabe(&stab_loc, entry);
    ls_add_int(&to_push, stab_idx);

    const char * name =  "__str";
    ls_addran_char(&strtab, name, strlen(name));
    str index = strgen_next();
    ls_addran_char(&strtab, index.data, index.len + 1);
}

void macho_relocent(fat f, stack_context *s, u32 symbolnum, enum reloc_type_arm64 type, bool pcrel) {
    ls_u32 *stackcode = &s->code;
    const struct relocation_info rel = {
        .r_address = stackcode->count * sizeof (u32),
        .r_symbolnum = symbolnum,

        .r_pcrel = pcrel,
        .r_length = 2,
        .r_extern = true,
        .r_type = type,
    };
    ls_add_u32(&s->relocents, relocents.count);
    ls_add_relocent(&relocents, rel);
    printd("relocent %d, r_address: 0x%x\n", relocents.count, rel.r_address);
}

void macho_relocent_undef(fat f, stack_context *s, u32 symbolnum, enum reloc_type_arm64 type, bool pcrel) {
    ls_u32 *stackcode = &s->code;
    struct relocation_info rel = {
        .r_address = stackcode->count * sizeof (u32),
        .r_symbolnum = symbolnum,

        .r_pcrel = 1,
        .r_length = 2,
        .r_extern = 1,
        .r_type = ARM64_RELOC_BRANCH26
    };
    ls_add_int(&to_push_und, relocents.count);
    ls_add_u32(&s->relocents, relocents.count);
    ls_add_relocent(&relocents, rel);

}

typedef struct {
    char *find;
    int symbolnum;
} tab_find;
tab_find stab_search(ls_stabe *tab, str token) {
    char *find = NULL;
    u32 symbolnum = tab->count;
    for (int i = 0; i < tab->count; ++i) { // have to also find in local stab
        stabe s = tab->data[i];
        u32 index = s.n_un.n_strx;
        if (str_equal_c(token, strtab.data + index)) {
            find = strtab.data + index;
            symbolnum = i;
        }
    }
    return (tab_find){ find, symbolnum };
}

