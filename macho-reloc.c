#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach-o/loader.h>

#include "symbols.c"
#include "opcode.c"

void compile_err(const char *s, ...) { 
    va_list argptr;
    va_start(argptr, s);
    vfprintf(stderr, s, argptr);
    va_end(argptr);
    exit(1);
}
#define IS_DIGIT(c) (c >= '0' && c <= '9')
#define IS_LETTER(c) (c >= 'A' && c <= 'z')
#define OPC(o, op) list_add_uint32_t(o, op); cur_pc += sizeof (uint32_t);


// parser context
struct list_uint32_t opc_all;
char *strlits;
int strlit_idx = 0;
int strlit_cap = INIT_CAP;
struct _symbols symbols;
struct list_resolve_data to_resolve;
char *srcbuf;
size_t filelen;
uint64_t cur_pc = 0;
int reg = 0;
size_t i;
char c;
struct {
    size_t stacksiz;
    bool callsub;
    struct list_uint32_t opc;
    bool inrt;
} rtinfo;
// end

void rst_rtinfo(void) {
    rtinfo.opc.count = 0;
    rtinfo.callsub = false;
    rtinfo.stacksiz = 0;
}

void endofrt(void) {
    if (rtinfo.stacksiz == 0 && rtinfo.callsub) {
        uint32_t *last = rtinfo.opc.data + rtinfo.opc.count - 1;
        if (*last == RET) {
            *last = LDP_PREL;
            list_add_uint32_t(&rtinfo.opc, RET);
        } else {
            list_add_uint32_t(&rtinfo.opc, LDP_PREL);
        }
        cur_pc += sizeof (uint32_t);
    }
    list_addrang_uint32_t(&opc_all, rtinfo.opc.data, rtinfo.opc.count);
    printf("\nend of routine, stacksiz %zu\n", rtinfo.stacksiz);
}

void letter(void) {
    size_t tok_start = i;
    while ((c = srcbuf[i])) {
        if (c == ' ' || c == '\n') {
            srcbuf[i] = '\0';
            char *tmp = srcbuf + tok_start;
            if (strcmp(tmp, "ret") == 0) {
                list_add_uint32_t(&rtinfo.opc, RET);
                cur_pc += sizeof (uint32_t);
                printf("ret ");
            } else {
                OPC(&rtinfo.opc, ADRP);
                struct _resolve_data rd = { .addr = cur_pc, .str = tmp };
                list_add_resolve_data(&to_resolve, rd);
                OPC(&rtinfo.opc, ADD_IMM);
                printf("adrpadd(%s) ", tmp);
            }
            break;
        } else if (c != ':') {
            ++i;
            continue;
        }
        if (rtinfo.inrt) {
            endofrt();
            rtinfo.inrt = false;
        }
        srcbuf[i] = '\0';
        struct symbol s;
        s.p = srcbuf + tok_start;
        s.addr = cur_pc;
        symbols_add(&symbols, s);
        printf("\nsym(%s @0x%x) ", s.p, s.addr);
        printf("start of routine\n");
        rst_rtinfo();
        rtinfo.inrt = true;
        break;
    }
}

void digit(void) {
    size_t tok_start = i;
    while ((c = srcbuf[i])) {
        if (IS_DIGIT(c)) {
            ++i;
            continue;
        }
        if (c == ',') {
            if (reg >= 8)
                compile_err("%s", "used up all registers.\n");
            ++reg;
        }
        srcbuf[i] = '\0';
        long lit = strtol(srcbuf + tok_start, NULL, 10);
        uint32_t op = MOV | lit << 5 | reg;
        printf("mov(w0 %ld->%x) ", lit, op);
        list_add_uint32_t(&rtinfo.opc, op);
        cur_pc += sizeof (uint32_t);
        break;
    }
}

void readsym() {
    c = srcbuf[i++];
    if (!(IS_LETTER(c))) {
        compile_err("a symbol must start with a letter: '%s'\n", srcbuf + i - 3);
    }
    while ((c = srcbuf[i])) {
        if (IS_LETTER(c) || IS_DIGIT(c)) {
            ++i; continue;
        }
        break;
    }
}

void check_callsub(void) {
    if (!rtinfo.callsub) {
        list_add_uint32_t(&opc_all, STP_PREL);
        cur_pc += sizeof (uint32_t);
        rtinfo.callsub = true;
    }
}

void hyphen(bool linked) {
    char next = srcbuf[++i];
    if (next == '>') {
        size_t start_pos = ++i;
        readsym();
        srcbuf[i] = '\0';
        char *str = srcbuf + start_pos;
        uint32_t op;
        if (linked) {
            op = BL;
            printf("bl(%s) ", str);
            check_callsub();

        } else {
            op = B;
            printf("b(%s) ", str);
        }
        list_add_uint32_t(&rtinfo.opc, op);
        struct _resolve_data tmp = { .addr = cur_pc, .str = str };
        list_add_resolve_data(&to_resolve, tmp);
        cur_pc += sizeof (uint32_t);
    }
}

void strlit_add(void) {
    c = srcbuf[i++];
    size_t start_pos = i;
    while ((c = srcbuf[i])) {
        if (c != '"') {
            ++i; continue;
        }
        break;
    }
    if (c != '"')
        compile_err("%s", "string literal not terminated\n");
    srcbuf[i] = '\0';
    const char *s = srcbuf + start_pos;
    printf("string lit (%s) ", s);
    unsigned long len = strlen(s);
    size_t nitems = (len + 1) / (sizeof (uint32_t));
    list_addrang_uint32_t(&rtinfo.opc, (uint32_t *)s, nitems);
    /* if (strlit_idx + len > strlit_cap) { */
    /*     strlit_cap += 0x4; */
    /*     strlits = reallocf(strlits, strlit_cap); */
    /* } */
    /* strlcpy(strlits + strlit_idx, s, strlit_cap - strlit_idx); */
    cur_pc += nitems * sizeof (uint32_t);
}

void parse(void) {
    for (i = 0; i < filelen - 1; ++i) {
        c = srcbuf[i];
        if (IS_LETTER(c)) {
            letter();
        } else if (IS_DIGIT(c)) {
            digit();
        } else if (c == '-' || c == '=') {
            hyphen(c == '=');
        } else if (c == '"') {
            strlit_add();
        }
    }
}

void resolve_symbols(void) {
    for (int i = 0; i < to_resolve.count; ++i) {
        struct _resolve_data d = to_resolve.data[i];
        printf("try(%s, %x) ", d.str, d.addr);
        struct symbol *s;
        int j;
        bool found = false;
        for (j = 0; j < symbols.count; ++j) {
            s = symbols.data + j;
            if (strcmp(s->p, d.str) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            compile_err("could not find symbol '%s'\n", d.str);
            return;
        }
        size_t idx = d.addr / sizeof (uint32_t);
        switch (opc_all.data[idx]) {
        case B:
        case BL:;
            uint32_t dif = s->addr - d.addr;
            opc_all.data[idx] = opc_all.data[idx] | (dif / sizeof (uint32_t));
            printf("resolv(%s->%x, %x) ", d.str, dif, opc_all.data[idx]);
            break;
        case ADD_IMM:
            opc_all.data[idx] |= ((s->addr + 0xf80) << 10);
            printf("resolv(%s, %x) ", d.str, opc_all.data[idx]);
            break;
        default:
            compile_err("undefined opcode %x\n", opc_all.data[idx]);
            break;
        }
    }
}

int main(void) {
    size_t total_size = 0;
    uint64_t textsect_size = 0;

// #parser
    FILE *src = fopen("src.al", "r");
    if (src == NULL) {
        return 1;
    }
    fseek(src, 0L, SEEK_END);
    filelen = ftell(src);
    fseek(src, 0L, SEEK_SET);

    srcbuf = malloc(filelen);
    if (srcbuf == NULL) {
        return 2;
    }
    fread(srcbuf, sizeof (char), filelen, src);
    fclose(src);

    list_new_uint32_t(&opc_all);
    list_new_uint32_t(&rtinfo.opc);
    list_new_resolve_data(&to_resolve);
    symbols_new(&symbols);
    strlits = malloc(INIT_CAP);

    printf(":: parse start\n");
    parse();
    endofrt();
    printf("\n:: parse end\n");
    resolve_symbols();
    printf("\n:: resolv end\n");
    total_size += strlit_cap;
    textsect_size += strlit_cap;

    struct list_uint64_t symtab_data;
    list_new_uint64_t(&symtab_data);

    int strtab_size = 0x10;
    char *strtab = malloc(strtab_size * sizeof (char));
    strtab[0] = '\0';
    int strtab_idx = 1;

    for (int i = 0; i < symbols.count; ++i) {
        struct symbol data = symbols.data[i];
        char *s = data.p;
        unsigned long len = strlen(s);
        while (strtab_idx + len > strtab_size) {
            strtab_size += 0x10;
            strtab = reallocf(strtab, strtab_size);
        }
        strlcpy(strtab + strtab_idx, s, strtab_size - strtab_idx);
        uint64_t flag = 0x010f00000000;
        list_add_uint64_t(&symtab_data, flag | strtab_idx);
        list_add_uint64_t(&symtab_data, data.addr);
        strtab_idx += strlen(s) + 1;
    }

    int symtab_data_size = symtab_data.count * sizeof (uint64_t);
    total_size += symtab_data_size;
    total_size += strtab_size;

    free(srcbuf);

// # opc
    if (opc_all.count % 2 == 1)
        list_add_uint32_t(&opc_all, 0);
    int opc_size = opc_all.count * sizeof (uint32_t);
    total_size += opc_size;
    textsect_size += opc_size;
    printf("textsect %llx", textsect_size);


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
    dysymtab.nlocalsym = symbols.count;
    dysymtab.iextdefsym = dysymtab.ilocalsym + dysymtab.nlocalsym;
    dysymtab.nextdefsym = 0x0;

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
    // add opc
    for (int i = 0; i < opc_all.count; ++i) {
        uint32_t *ptr = (uint32_t *)buf;
        *ptr = opc_all.data[i];
        buf += sizeof (uint32_t);
    }
    // add strlits
    memcpy(buf, strlits, strlit_cap);
    buf += strlit_cap;
    // add symtab data
    memcpy(buf, symtab_data.data, symtab_data_size);
    buf += symtab_data_size;
    // add strtab
    memcpy(buf, strtab, strtab_size);
    buf += strtab_size;

    FILE *file = fopen("machoreloc.out", "w");
    fwrite(buf_base, sizeof (char), total_size, file);
    fclose(file);
    free(buf_base);
    symbols_delete(&symbols);
    list_delete_uint32_t(&opc_all);
    list_delete_resolve_data(&to_resolve);
    list_delete_uint64_t(&symtab_data);
}

