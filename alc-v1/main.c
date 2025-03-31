#include <stdio.h>
#include <stdlib.h>

#include "file.h"
#include "macho.h"
#include "parser.h"
#include "typedefs.h"

int main(void) {
    str src = read_source();

    parse(src);

    free(src.data);

// need to parse out
    size_t objcode_size = ((u32 *)objcode - _objcode) * sizeof (u32);
    //
    //printf("%zd", objcode_size);
    slice asm_out = slice_new(_objcode, objcode_size);

    int nreloc = 0;

    int nsyms = 2;
    int nlocal_syms = 1;
    int nextdef_syms = 1;
    int nundef_syms = 0;

    // symtab
    struct nlist_64 entry0 = {
        .n_un.n_strx = 7,
        .n_type = 0x0e,
        .n_sect = 0x1,
        .n_desc = 0x0,
        .n_value = 0L,
    };

    struct nlist_64 entry1;
    entry1.n_un.n_strx = 1; // TODO
    entry1.n_type = 0x0f; // TODO
    entry1.n_sect = 0x1; // TODO
    entry1.n_desc = 0x0; // TODO
    entry1.n_value = 0L; // TODO

    // strtab

    slice strtab = slice_new("\0_main\0ltmp0\0\0\0\0", 0x10);

    // reloctab
    u32 relocoff = 0;

// prepare mach-o obj file
    header_init(&s_header);

    lc_segment_c seg_text;
    lc_segment_begin(&seg_text, SEG_TEXT);
    lc_segment_sect_info info = {
        .name = SECT_TEXT,
        .size = asm_out.size,
        .relocoff = relocoff,
        .nreloc = nreloc,
    };
    lc_segment_sect(&seg_text, &s_lcs.section_text, &info);
    lc_segment_end(&seg_text, &s_lcs.segment);

    lc_build_version(&s_lcs.build_ver);
    lc_symtab(&s_lcs.symtab, nsyms, strtab.size);
    lc_dysymtab(&s_lcs.dysymtab, nlocal_syms, nextdef_syms, nundef_syms);


// calculate offsets

    long offset = 0;
    offset += sizeof (struct mach_header_64);

    s_header.ncmds = s_num_cmds;
    s_header.sizeofcmds = sizeof (load_commands);
    offset += s_header.sizeofcmds;

    s_lcs.segment.fileoff = offset;
    s_lcs.section_text.offset = offset;
    offset += asm_out.size;

    s_lcs.symtab.symoff = offset;
    offset += (s_lcs.symtab.nsyms * sizeof (struct nlist_64));

    s_lcs.symtab.stroff = offset;
    offset += s_lcs.symtab.strsize;

    long total_size = offset;

// write to file
    void *obj_buf = malloc(total_size);
    writer_t ptr = obj_buf;

    get_slice(s_header);

    write_buf2(&ptr, get_slice(s_header));
    write_buf2(&ptr, get_slice(s_lcs));
    write_buf2(&ptr, asm_out);
    write_buf2(&ptr, get_slice(entry0));
    write_buf2(&ptr, get_slice(entry1));
    write_buf2(&ptr, strtab);
    // TODO add relocent

    FILE *file = fopen("main.o", "w");
    fwrite(obj_buf, sizeof (char), total_size, file);
    fclose(file);
    free(obj_buf);
}

