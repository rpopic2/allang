#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach-o/loader.h>

#include "symbols.c"
#define OPC_RET 0xd65f03c0

#define compile_err(s) printf(s); \
    exit(1);\

int main(void) {
    size_t total_size = 0;
    uint64_t textsect_size = 0;

// #parser
    FILE *src = fopen("src.al", "r");
    if (src == NULL) {
        return 1;
    }
    fseek(src, 0L, SEEK_END);
    size_t filelen = ftell(src);
    fseek(src, 0L, SEEK_SET);

    char *srcbuf = malloc(filelen);
    if (srcbuf == NULL) {
        return 2;
    }
    fread(srcbuf, sizeof (char), filelen, src);
    fclose(src);

    struct list_uint32_t opc;
    list_new_uint32_t(&opc);

    symbols_new(&symbols);
    int cur_pc = 0;
    int reg = 0;

    printf(":: sym parse start\n");
    for (size_t i = 0; i < filelen - 1; ++i) {
        char c = srcbuf[i];
        // printf("%zd ", i);
        bool isLetter = c >= 'A' && c <= 'z';
        if (isLetter) {
            size_t tok_start = i;
            while ((c = srcbuf[i])) {
                if (c == ' ' || c == '\n') {
                    size_t count = i - tok_start + 1;
                    char tmp[count];
                    strlcpy(tmp, srcbuf + tok_start, count);
                    printf("(%s)", tmp);
                    if (strcmp(tmp, "ret") == 0) {
                        list_add_uint32_t(&opc, OPC_RET);
                    }
                    break;
                } else if (c != ':') {
                    ++i;
                    continue;
                }
                srcbuf[i] = '\0';
                struct symbol s;
                s.p = srcbuf + tok_start;
                s.addr = cur_pc;
                symbols_add(&symbols, s);
                printf("sym(%s @0x%x) ", s.p, s.addr);
                break;
            }
        }
#define IS_DIGIT(c) c >= '0' && c <= '9'
        if (IS_DIGIT(c)) {
            size_t tok_start = i;
            while ((c = srcbuf[i])) {
                if (IS_DIGIT(c)) {
                    ++i;
                    continue;
                }
                if (srcbuf[i] == ',') {
                    if (reg >= 8) {
                        compile_err("used up all registers.\n");
                    }
                    ++reg;
                }
                srcbuf[i] = '\0';
                long lit = strtol(srcbuf + tok_start, NULL, 10);
                uint32_t op = 0x52800000 | lit << 5 | reg;
                printf("mov(w0 %ld->%x) ", lit, op);
                list_add_uint32_t(&opc, op);
                cur_pc += sizeof (uint32_t);
                break;
            }
        }
    }

    printf("\n:: sym parse end\n");

    struct list_uint64_t symtab_data;
    list_new_uint64_t(&symtab_data);


    int strtab_size = 0x10;
    char *strtab = malloc(strtab_size * sizeof (char));
    strtab[0] = '\0';
    int idx = 1;

    for (int i = 0; i < symbols.count; ++i) {
        struct symbol data = symbols.data[i];
        char *s = data.p;
        strlcpy(strtab + idx, s, strtab_size - idx);\
        uint64_t flag = 0x010f00000000;
        list_add_uint64_t(&symtab_data, flag | idx);
        list_add_uint64_t(&symtab_data, data.addr);
        idx += strlen(s) + 1;
    }

    int symtab_data_size = symtab_data.count * sizeof (uint64_t);

    free(srcbuf);

// # opc
    int opc_size = opc.count * sizeof (uint32_t);
    total_size += opc_size;
    textsect_size += opc_size;
    printf("textsect %llx", textsect_size);

// # symtab data
    /* uint64_t symtab_data[] = { */
    /*     0x010e00000007, 0x0, */
    /*     0x010f00000001, 0x0, */
    /* }; */
    total_size += symtab_data_size;

    total_size += strtab_size;


    uint32_t load_cmds_num = 0;
    uint32_t load_cmds_size = 0;
#define LC_REG(lc) ++load_cmds_num; load_cmds_size += lc.cmdsize; \
        printf("%s %x (%x)\n", #lc, load_cmds_size, lc.cmdsize);

// # lc segload
    struct segment_command_64 segload;
    segload.cmd = LC_SEGMENT_64;
    segload.cmdsize = sizeof segload + sizeof (struct section_64);
    LC_REG(segload);
    const char GPNAME[sizeof segload.segname] = "";

    memcpy(segload.segname, GPNAME, sizeof segload.segname);
    segload.vmaddr = 0x0;

    segload.vmsize = textsect_size;

    segload.filesize = textsect_size;
    segload.maxprot = 0x7;
    segload.initprot = 0x7;

    segload.nsects = 0x1;
    segload.flags = 0x0;

    // ## sect
    struct section_64 sect;
#define STR(dst, src) \
    const char src ## _str[sizeof dst] = #src; \
    memcpy(dst, src ## _str, sizeof dst);
    STR(sect.sectname, __text)
    STR(sect.segname, __TEXT)
    sect.addr = 0;

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
    symtab.nsyms = symbols.count;
    symtab.strsize = strtab_size;

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

    // # header
    struct mach_header_64 header;
    total_size += sizeof header;

    header.magic = MH_MAGIC_64;
    header.cputype = 0x0100000c;
    header.cpusubtype = 0;
    header.filetype = 0x1;

    header.ncmds = load_cmds_num;
    header.sizeofcmds = load_cmds_size; //0x118?
    header.flags = 0x0;
    header.reserved = 0x0;

// calc offsets
    sect.offset = segload.fileoff = sizeof header + header.sizeofcmds;
    symtab.symoff = sect.offset + textsect_size;
    symtab.stroff = symtab.symoff + symtab_data_size;
    total_size += load_cmds_size;

// buffer

    void *buf = malloc(total_size);
    void *buf_base = buf;

#define ADD(value)\
    memcpy(buf, &value, sizeof value); \
    buf += sizeof value;
    // printf(#value" %p (+%zd)\n", buf, sizeof value);

    ADD(header)
    ADD(segload)
    ADD(sect)
    ADD(ver)
    ADD(symtab)
    ADD(dysymtab)
    for (int i = 0; i < opc.count; ++i) {
        uint32_t *ptr = (uint32_t *)buf;
        *ptr = opc.data[i];
        buf += sizeof (uint32_t);
    }
    memcpy(buf, symtab_data.data, symtab_data_size);
    buf += symtab_data_size;
    memcpy(buf, strtab, strtab_size);
    buf += strtab_size;

    FILE *file = fopen("machoreloc.out", "w");
    fwrite(buf_base, sizeof (char), total_size, file);
    fclose(file);
    free(buf_base);
    symbols_delete(&symbols);
    list_delete_uint32_t(&opc);
    list_delete_uint64_t(&symtab_data);
}

