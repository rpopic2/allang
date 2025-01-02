#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach-o/loader.h>

int main(void) {
    size_t total_size = 0;
    uint64_t textsect_size = 0;

// # opc
    uint32_t opc[] = { 0x52800020, 0xd65f03c0 };
    total_size += sizeof opc;
    textsect_size += sizeof opc;

// # symtab data
    uint64_t symbols[] = {
        0x010e00000007, 0x0,
        0x010f00000001, 0x0,
    };
    total_size += sizeof symbols;

    const char strtab[0x10] = "\0_main\0ltmp0";
    total_size += sizeof strtab;


    uint32_t load_cmds_num = 0;
    uint32_t load_cmds_size = 0;
#define LC_REG(lc) ++load_cmds_num; load_cmds_size += lc.cmdsize; \
        printf("%s %x (%x)\n", #lc, load_cmds_size, lc.cmdsize);

// # lc segload pagezero
    struct segment_command_64 segload_pz;
    segload_pz.cmd = LC_SEGMENT_64;
    segload_pz.cmdsize = sizeof segload_pz;
    LC_REG(segload_pz);

    const char GPNAME_PZ[sizeof segload_pz.segname] = "__PAGEZERO";
    memcpy(segload_pz.segname, GPNAME_PZ, sizeof segload_pz.segname);
    segload_pz.vmaddr = 0x0;

    segload_pz.vmsize = 0x0100000000;

    segload_pz.filesize = 0;
    segload_pz.maxprot = 0x0;
    segload_pz.initprot = 0x0;

    segload_pz.nsects = 0x0;
    segload_pz.flags = 0x0;

// # lc segload text
    struct segment_command_64 segload;
    segload.cmd = LC_SEGMENT_64;
    segload.cmdsize = sizeof segload + sizeof (struct section_64);
    LC_REG(segload);

#define STR(dst, src) \
    const char src ## _str[sizeof dst] = #src; \
    memcpy(dst, src ## _str, sizeof dst);
    const char GPNAME[sizeof segload.segname] = "__TEXT";

    memcpy(segload.segname, GPNAME, sizeof segload.segname);
    segload.vmaddr = segload_pz.vmsize;

    segload.vmsize = textsect_size;

    segload.filesize = textsect_size;
    segload.maxprot = 0x5;
    segload.initprot = 0x5;

    segload.nsects = 0x1;
    segload.flags = 0x0;

    // ## sect
    struct section_64 sect;
    STR(sect.sectname, __text)
    STR(sect.segname, __TEXT)
    sect.addr = 0x0100003f78;

    sect.size = textsect_size;
    // sect.offset = 0x138;
    sect.align = 0x2;

    sect.reloff = 0x0;
    sect.nreloc = 0x0;
    sect.flags = 0x80000400;
    sect.reserved1 = 0x0;
    sect.reserved2 = 0x0;
    sect.reserved3 = 0x0;

// # lc build_version
    struct build_version_command ver;

    ver.cmd = LC_BUILD_VERSION;
    ver.cmdsize = sizeof ver;
    LC_REG(ver);

    ver.platform = PLATFORM_MACOS;
    ver.minos = 0x000e0000;
    ver.sdk = 0;
    ver.ntools = 0;

// # lc symtab
    struct symtab_command symtab;
    symtab.cmd = LC_SYMTAB;
    symtab.cmdsize = sizeof symtab;
    LC_REG(symtab);
    symtab.nsyms = 0x2;
    symtab.strsize = 0x10;

// # lc dysymtab
    struct dysymtab_command dysymtab;
    memset(&dysymtab, 0, sizeof dysymtab);
    dysymtab.cmd = LC_DYSYMTAB;
    dysymtab.cmdsize = sizeof dysymtab;
    LC_REG(dysymtab);

    dysymtab.ilocalsym = 0x0;
    dysymtab.nlocalsym = 0x1;
    dysymtab.iextdefsym = dysymtab.ilocalsym + dysymtab.nlocalsym;
    dysymtab.nextdefsym = 0x1;

    dysymtab.iundefsym = dysymtab.iextdefsym + dysymtab.nextdefsym;
    dysymtab.nundefsym = 0x0;

// # lc load dyld
    struct dylinker_command lddyld;
    lddyld.cmd = LC_LOAD_DYLINKER;
    lddyld.cmdsize = sizeof lddyld;
    lddyld.name.offset = 0x0c;
    LC_REG(lddyld);

    // ## dyld path
    const char dyldpath[0x14] = "/usr/lib/dyld";
    load_cmds_size += sizeof dyldpath;

// # header
    struct mach_header_64 header;
    total_size += sizeof header;

    header.magic = MH_MAGIC_64;
    header.cputype = 0x0100000c;
    header.cpusubtype = 0;
    header.filetype = MH_EXECUTE;

    header.ncmds = load_cmds_num;
    header.sizeofcmds = load_cmds_size; //0x118?
    header.flags = 0x0;
    header.reserved = 0x0;

    sect.offset = segload.fileoff = sizeof header + header.sizeofcmds;
    symtab.symoff = sect.offset + textsect_size;
    symtab.stroff = symtab.symoff + sizeof symbols;
    total_size += load_cmds_size;

// buffer

    void *buf = malloc(total_size);
    void *buf_base = buf;

#define ADD(value)\
    memcpy(buf, &value, sizeof value); \
    buf += sizeof value;

    ADD(header)
    ADD(segload_pz)
    ADD(segload)
    ADD(sect)
    ADD(ver)
    ADD(symtab)
    ADD(dysymtab)
    ADD(lddyld)
    ADD(dyldpath)
    ADD(opc)
    ADD(symbols)
    ADD(strtab)

    FILE *file = fopen("machoexe.out", "w");
    fwrite(buf_base, sizeof (char), total_size, file);
    fclose(file);
    free(buf_base);
}
