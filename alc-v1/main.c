#include <stdio.h>
#include <stdlib.h>
#include <mach-o/arm64/reloc.h>

#include "file.h"
#include "macho.h"
#include "parser.h"
#include "typedefs.h"

int main(void) {
    str src = read_source();

    parse(src);

    free(src.data);

    if (has_compile_err) {
        return 1;
    }

// need to parse out
    size_t objcode_size = ((u32 *)objcode - _objcode + 1) * sizeof (u32);
    //
    //printf("%zd", objcode_size);
    slice asm_out = slice_new(_objcode, objcode_size);

    int nsyms = 3;
    int nlocal_syms = 1;
    int nextdef_syms = 1;
    int nundef_syms = 1;

    struct nlist_64 entry_main = {
        .n_un.n_strx = 1,
        .n_type = N_EXT | N_TYPE,
        .n_sect = 0x1,
        .n_desc = 0x0,
        .n_value = 0L,
    };

    int nreloc = 3;
    // 08 00 00 00 01 00 00 3d - adrp
    struct relocation_info rel_adrp = {
        .r_address = 0x8,
        .r_symbolnum = 0, // symbol index

        .r_pcrel = true,
        .r_length = 2, // long
        .r_extern = true,
        .r_type = ARM64_RELOC_PAGE21,
    };

    // 0c 00 00 00 01 00 00 4c - add 
    struct relocation_info rel_add = {
        .r_address = 0xc,
        .r_symbolnum = 0, // symbol index

        .r_pcrel = 0,
        .r_length = 2,
        .r_extern = 1,
        .r_type = ARM64_RELOC_PAGEOFF12
    };

    // 10 00 00 00 05 00 00 2d
    struct relocation_info rel_printf = {
        .r_address = 0x10,
        .r_symbolnum = 2, // symbol index

        .r_pcrel = 1,
        .r_length = 2,
        .r_extern = 1,
        .r_type = ARM64_RELOC_BRANCH26
    };


// prepare mach-o obj file
    header_init(&s_header);

    lc_segment_c seg_text;
    lc_segment_begin(&seg_text, SEG_TEXT);
    lc_segment_sect_info info = {
        .name = SECT_TEXT,
        .size = asm_out.size,
        .nreloc = nreloc,
    };
    lc_segment_sect(&seg_text, &s_lcs.section_text, &info);
    lc_segment_end(&seg_text, &s_lcs.segment);

    lc_build_version(&s_lcs.build_ver);
    lc_symtab(&s_lcs.symtab, nsyms, strtab.count);
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

    s_lcs.section_text.reloff = offset;
    offset += (s_lcs.section_text.nreloc * sizeof (struct relocation_info));

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

    write_buf2(&ptr, get_slice(rel_adrp));
    write_buf2(&ptr, get_slice(rel_add));
    write_buf2(&ptr, get_slice(rel_printf));

    write_buf2(&ptr, get_slice(stab_loc.data[0]));
    write_buf2(&ptr, get_slice(entry_main));
    write_buf2(&ptr, get_slice(stab_und.data[0]));

    write_buf2(&ptr, (slice) {.data =strtab.data, .size = strtab.count });

    FILE *file = fopen("main.o", "w");
    fwrite(obj_buf, sizeof (char), total_size, file);
    fclose(file);
    free(obj_buf);
}

