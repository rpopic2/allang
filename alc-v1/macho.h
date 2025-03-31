#pragma once

#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach/machine.h>
#include <string.h>
#include "typedefs.h"

typedef struct {
    struct segment_command_64 segment;
    struct section_64 section_text; // not a lc
    struct build_version_command build_ver;
    struct symtab_command symtab;
    struct dysymtab_command dysymtab;
} load_commands;

struct mach_header_64 s_header;
load_commands s_lcs;
int s_num_cmds = 0;

void header_init(struct mach_header_64 *header) {
    header->magic = MH_MAGIC_64;
    header->cputype = CPU_TYPE_ARM64;
    header->cpusubtype = CPU_SUBTYPE_LITTLE_ENDIAN;
    header->filetype = MH_OBJECT;

    // header->flags = 0;
    // header->reserved = 0;
}

typedef struct {
    const char *name;
    u32 nsects;
    u64 size;
} lc_segment_c;

typedef struct {
    const char *name;
    u64 size;
    u32 relocoff;
    u32 nreloc;
} lc_segment_sect_info;

void lc_segment_begin(lc_segment_c *self, const char *segname) {
    self->name = segname;
    self->nsects = 0;
    self->size = 0L;
}

void lc_segment_sect(lc_segment_c *self, struct section_64 *sect, lc_segment_sect_info *info) {
    ++self->nsects;

    strcpy(sect->sectname, info->name);
    strcpy(sect->segname, self->name);
    sect->addr = 0L;
    sect->size = info->size;
    self->size += info->size;

    // offset calculated later
    sect->align = 2;
    sect->reloff = info->relocoff,
    sect->nreloc = info->nreloc;
    sect->flags = 0x80000400; // 8: pure_instrunctions, 4: contains some instructions, type is S_REGULAR
}

void lc_segment_end(lc_segment_c *self, struct segment_command_64 *seg) {
    ++s_num_cmds;
    seg->cmd = LC_SEGMENT_64;
    seg->cmdsize = sizeof (struct segment_command_64) + sizeof (struct section_64);
    // segname will be zeroed out
    // vmaddr is 0

    seg->vmsize = self->size;
    seg->filesize = self->size;
    seg->maxprot = 7; //rwx
    seg->initprot = 7;
    seg->nsects = self->nsects;
    // flags be 0
}
void lc_build_version(struct build_version_command *build_ver) {
    ++s_num_cmds;
    build_ver->cmd = LC_BUILD_VERSION;
    build_ver->cmdsize = sizeof (struct build_version_command);
    build_ver->platform = PLATFORM_MACOS;
    build_ver->minos = 0x0e0000;
}

void lc_symtab(struct symtab_command *symtab, u32 nsyms, u32 strsize) {
    ++s_num_cmds;
    symtab->cmd = LC_SYMTAB;
    symtab->cmdsize = sizeof (struct symtab_command);
    symtab->nsyms = nsyms;
    symtab->strsize = strsize;
    // symoff calculated later
    // stroff calculated later
}

void lc_dysymtab(struct dysymtab_command *symtab, u32 nlocal_syms, u32 nextdef_syms, u32 nundef_syms) {
    ++s_num_cmds;
    symtab->cmd = LC_DYSYMTAB;
    symtab->cmdsize = sizeof (struct dysymtab_command);

    int dysymtab_idx = 0;

    symtab->ilocalsym = 0;
    symtab->nlocalsym = nlocal_syms;
    dysymtab_idx += nlocal_syms;

    symtab->iextdefsym = dysymtab_idx;
    symtab->nextdefsym = nextdef_syms;
    dysymtab_idx += nextdef_syms;

    symtab->iundefsym = dysymtab_idx;
    symtab->nundefsym = nundef_syms;
}

