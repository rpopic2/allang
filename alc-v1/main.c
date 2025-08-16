#include <stdio.h>
#include <stdlib.h>
#include <mach-o/arm64/reloc.h>

#include "file.h"
#include "macho.h"
#include "parser.h"
#include "typedefs.h"

int main(int argc, const char **argv) {
    str src;
    if (argc == 1) {
        src = read_source("main.al");
    } else {
        src = read_source(argv[1]);
    }

    parse(src);

    free(src.data);

    if (has_compile_err) {
        CompileErr("Compile Error\n");
        return 1;
    }

    size_t objcode_size = ((u32 *)objcode - _objcode + 1) * sizeof (u32);

    slice asm_out = slice_new(_objcode, objcode_size);

    int nreloc = relocents.count;


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
    int nsyms = stab_loc.count + stab_ext.count + stab_und.count;
    lc_symtab(&s_lcs.symtab, nsyms, strtab.count);
    lc_dysymtab(&s_lcs.dysymtab, stab_loc.count, stab_ext.count, stab_und.count);

// push relocents
    for (int i = 0; i < to_push_ext.count; ++i) {
        int index = to_push_ext.data[i];
        // int original = relocents.data[index].r_symbolnum;
        relocents.data[index].r_symbolnum += s_lcs.dysymtab.iextdefsym;
        // printf("pushed ext %d, %d->%d\n", index, original, relocents.data[index].r_symbolnum);
    }
    for (int i = 0; i < to_push_und.count; ++i) {
        int index = to_push_und.data[i];
        // int original = relocents.data[index].r_symbolnum;
        relocents.data[index].r_symbolnum += s_lcs.dysymtab.iundefsym;
        // printf("pushed und %d, %d->%d\n", index, original, relocents.data[index].r_symbolnum);
    }

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

    for (int i = 0; i < relocents.count; ++i) {
        write_buf2(&ptr, get_slice(relocents.data[i]));
    }
    // write_buf2(&ptr, get_slice(rel_printf));

    for (int i = 0; i < stab_loc.count; ++i) {
        write_buf2(&ptr, get_slice(stab_loc.data[i]));
    }
    for (int i = 0; i < stab_ext.count; ++i) {
        stabe d = stab_ext.data[i];
        write_buf2(&ptr, get_slice(d));
    }
    for (int i = 0; i < stab_und.count; ++i) {
        write_buf2(&ptr, get_slice(stab_und.data[i]));
    }

    write_buf2(&ptr, (slice) {.data =strtab.data, .size = strtab.count });

    FILE *file = fopen("main.o", "w");
    fwrite(obj_buf, sizeof (char), total_size, file);
    fclose(file);
    free(obj_buf);
}

