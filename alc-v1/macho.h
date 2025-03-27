#pragma once

#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach/machine.h>
#include <string.h>

typedef struct {
    struct segment_command_64 segment;
    struct section_64 section_text; // not a lc
    struct build_version_command build_ver;
    struct symtab_command symtab;
    struct dysymtab_command dysymtab;
} load_commands ;

// typedef struct {
//     struct nlist_64 entry;
//     const char *name;
// } symtab_pair;

void reloc_obj_new(struct mach_header_64 *header) {
    header->magic = MH_MAGIC_64;
    header->cputype = CPU_TYPE_ARM64;
    header->cpusubtype = CPU_SUBTYPE_LITTLE_ENDIAN;
    header->filetype = MH_OBJECT;

    header->flags = 0;
    header->reserved = 0;
}
