#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "macho.h"
#include "parser.h"

struct mach_header_64 header;
load_commands lcs;

int main(void) {
    str src = read_source();    // leaking src.data is intentional

    long asm_out = 0xd65f03c052800000;
    int asm_len = 8; // TODO from parse
    int nreloc = 0;

    int nsyms = 2;

    const char *strtab = "\0_main\0ltmp0\0\0\0\0";
    int strtab_size = 0x10;

    int nlocal_syms = 1;
    int nextdef_syms = 1;
    int nundef_syms = 0;


// prepare cmds
    reloc_obj_new(&header);
    int num_cmds = 0;

// LC_SEGMENT_64
    ++num_cmds;

    // __TEXT
    strcpy(lcs.section_text.sectname, SECT_TEXT);
    strcpy(lcs.section_text.segname, SEG_TEXT);
    lcs.section_text.addr = 0L;
    lcs.section_text.size = asm_len;

    // offset calculated later
    lcs.section_text.align = 2;
    lcs.section_text.reloff = 0; // TODO need to make it later
    lcs.section_text.nreloc = nreloc;
    lcs.section_text.flags = 0x80000400; // 8: pure_instrunctions, 4: contains some instructions, type is S_REGULAR


    lcs.segment.cmd = LC_SEGMENT_64;
    lcs.segment.cmdsize = sizeof (struct segment_command_64) + sizeof (struct section_64);

    lcs.segment.vmsize = asm_len;
    lcs.segment.filesize = asm_len;
    lcs.segment.maxprot = 7; //rwx
    lcs.segment.initprot = 7;
    lcs.segment.nsects = 1;

// LC_BUILD_VERSION
    ++num_cmds;
    lcs.build_ver.cmd = LC_BUILD_VERSION;
    lcs.build_ver.cmdsize = sizeof (struct build_version_command);
    lcs.build_ver.platform = PLATFORM_MACOS;
    lcs.build_ver.minos = 0x0e0000;

// LC_SYMTAB
    ++num_cmds;
    lcs.symtab.cmd = LC_SYMTAB;
    lcs.symtab.cmdsize = sizeof (struct symtab_command);
    // symoff calculated later
    lcs.symtab.nsyms = nsyms;
    // stroff calculated later
    lcs.symtab.strsize = strtab_size;

// LC_DYSYMTAB
    ++num_cmds;
    lcs.dysymtab.cmd = LC_DYSYMTAB;
    lcs.dysymtab.cmdsize = sizeof (struct dysymtab_command);

    int dysymtab_idx = 0;

    lcs.dysymtab.ilocalsym = 0;
    lcs.dysymtab.nlocalsym = nlocal_syms;
    dysymtab_idx += nlocal_syms;

    lcs.dysymtab.iextdefsym = dysymtab_idx;
    lcs.dysymtab.nextdefsym = nextdef_syms;
    dysymtab_idx += nextdef_syms;

    lcs.dysymtab.iundefsym = dysymtab_idx;
    lcs.dysymtab.nundefsym = nundef_syms;

// symtab
    struct nlist_64 entry0;
    entry0.n_un.n_strx = 7; // TODO
    entry0.n_type = 0x0e; // TODO
    entry0.n_sect = 0x1; // TODO
    entry0.n_desc = 0x0; // TODO
    entry0.n_value = 0L; // TODO

    struct nlist_64 entry1;
    entry1.n_un.n_strx = 1; // TODO
    entry1.n_type = 0x0f; // TODO
    entry1.n_sect = 0x1; // TODO
    entry1.n_desc = 0x0; // TODO
    entry1.n_value = 0L; // TODO

// strtab


// calculate offsets

    long offset = 0;
    offset += sizeof (struct mach_header_64);

    header.ncmds = num_cmds;
    header.sizeofcmds = sizeof (load_commands);
    offset += header.sizeofcmds;

    lcs.segment.fileoff = offset;
    lcs.section_text.offset = offset;
    offset += asm_len;

    lcs.symtab.symoff = offset;
    offset += (lcs.symtab.nsyms * sizeof (struct nlist_64));

    lcs.symtab.stroff = offset;
    offset += lcs.symtab.strsize;

    long total_size = offset;

// actually write file
    void *obj_buf = malloc(total_size);
    void *ptr = obj_buf;

    size_t size;

    size = sizeof (header);
    memcpy(ptr, &header, size);
    ptr += size;

    size = sizeof (lcs);
    memcpy(ptr, &lcs, size);
    ptr += size;

    size = asm_len;
    memcpy(ptr, &asm_out, size);
    ptr += size;

    size = sizeof (struct nlist_64);
    memcpy(ptr, &entry0, size);
    ptr += size;

    size = sizeof (struct nlist_64);
    memcpy(ptr, &entry1, size);
    ptr += size;

    size = 0x10;
    memcpy(ptr, strtab, size);
    ptr += size;

    // add relocent, symtab, strtab

    FILE *file = fopen("main.o", "w");
    fwrite(obj_buf, sizeof (char), total_size, file);
    fclose(file);
    free(obj_buf);
}

