#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach-o/loader.h>

#define printf(...) \
    printf(__VA_ARGS__) \

// #define AL_DRAW_STACK

#include "types.c"
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
#define OPC(o, op) list_add_uint32_t(o, (op)); cur_pc += sizeof (uint32_t);

struct list_uint32_t relocent;

// parser context
int ident = 0;
int ident_before = 0;

char *blk_name = "";
char *blk_name_brk = "";
int blk_idx = 0;

struct list_uint32_t opc_all;
char *strlits;
int strlit_idx = 0;
int strlit_cap = INIT_CAP;
struct list_symbol_t symbols;
int nundefsyms = 0;
struct list_resolve_data to_resolve;
char *srcbuf;
size_t filelen;
uint64_t cur_pc = 0;
int reg = 0;
size_t i;
char c;
bool cmpmode = false;
struct list_object objects;
struct {
    size_t stacksiz;
    struct list_uint32_t opc;
    int to_resolve_start;
    int loc_symbol_start;
    bool inrt;
    bool callsub;
} rtinfo;
// end

void rst_rtinfo(void) {
    rtinfo.opc.count = 0;
    rtinfo.callsub = false;
    rtinfo.stacksiz = 0;
    cmpmode = false;
    objects.count = 0;
}

void draw_stack(void) {
#ifdef AL_DRAW_STACK
    printf("\n");
    for (int i = 0; i < objects.count; ++i) {
        struct _object *obj = objects.data + i;
        printf("|%x\t%s(%x)\n", obj->offset, obj->name, obj->size);
        int lines = obj->size / 2;
        for (int j = 1; j < lines; ++j) {
            printf("|\n");
        }
    }
#endif
}

void endofrt(void) {
    int push = 0;
    if (rtinfo.stacksiz == 0 && rtinfo.callsub) {
        OPC(&opc_all, STPPRE_PREL)
        OPC(&opc_all, ADD_IMM | (0x1f << 5) | (0x1d))

        uint32_t *last = rtinfo.opc.data + rtinfo.opc.count - 1;
        bool last_ret = *last == RET;
        if (last_ret)
            rtinfo.opc.count -= 1;
        push = 8;

        OPC(&rtinfo.opc, LDPPOST_PREL)
        if (last_ret)
            list_add_uint32_t(&rtinfo.opc, RET);
    } else if (rtinfo.stacksiz > 0) {
        printf("original stack(%zx) ", rtinfo.stacksiz);
        push = 4;
        if (rtinfo.callsub)
            rtinfo.stacksiz += 0x10;   // sizeof { fp, lr }
        rtinfo.stacksiz += 0x10 - (rtinfo.stacksiz % 0x10);
        OPC(&opc_all, SUB | (rtinfo.stacksiz << 10) | (0b1111111111))
        int stpat = (rtinfo.stacksiz - 0x10) / 8;
        if (rtinfo.callsub) {
            OPC(&opc_all, STP_PREL | (stpat << 15))
            OPC(&opc_all, ADD_IMM | (0x10 << 0xa) | (31 << 5) | (29))
            push += 8;
        }

        uint32_t *last = rtinfo.opc.data + rtinfo.opc.count - 1;
        bool last_ret = *last == RET;
        if (last_ret)
            rtinfo.opc.count -= 1;

        if (rtinfo.callsub) {
            OPC(&rtinfo.opc, LDP_PREL | (stpat << 15))
        }
        OPC(&rtinfo.opc, ADD_IMM | (rtinfo.stacksiz << 10 | (0b1111111111)))

        if (last_ret)
            list_add_uint32_t(&rtinfo.opc, RET);
    }
    for (int i = rtinfo.to_resolve_start; i < to_resolve.count; ++i) {
        to_resolve.data[i].addr += push;
        printf("push sym %s, %x\n", to_resolve.data[i].str, to_resolve.data[i].addr);
    }
    for (int i = rtinfo.loc_symbol_start; i < symbols.count; ++i) {
        symbols.data[i].addr += push;
    }

    draw_stack();
    rtinfo.to_resolve_start = to_resolve.count;
    list_addrang_uint32_t(&opc_all, rtinfo.opc.data, rtinfo.opc.count);
    printf("\nend of routine, stacksiz %zx, sub %i\n", rtinfo.stacksiz, rtinfo.callsub);
}

void handle_reg(char c) {
    if (c == ',') {
        if (reg >= 8)
            compile_err("%s", "used up all registers.\n");
        ++reg;
    } else if (c == '\n') {
        reg = 0;
    }
}

void readsym() {
    c = srcbuf[i++];
    if (!(IS_LETTER(c))) {
        compile_err("a symbol must start with a letter: (but was '%c') @'%s'\n", c, srcbuf + i - 3);
    }
    while ((c = srcbuf[i])) {
        if (IS_LETTER(c) || IS_DIGIT(c)) {
            ++i; continue;
        }
        break;
    }
}

void letter(void) {
    size_t tok_start = i;
    readsym();
    char end = srcbuf[i];
    srcbuf[i] = '\0';

    if (end == ' ' || end == '\n' || end == ',') {
        char *tmp = srcbuf + tok_start;
        if (strcmp(tmp, "ret") == 0) {
            list_add_uint32_t(&rtinfo.opc, RET);
            cur_pc += sizeof (uint32_t);
            printf("ret ");
        } else {
            OPC(&rtinfo.opc, ADRP | reg);
            struct _resolve_data rd = { .addr = cur_pc, .str = tmp };
            list_add_resolve_data(&to_resolve, rd);
            OPC(&rtinfo.opc, ADD_IMM | (reg << 5) | reg);
            printf("adrpadd(%s @%x) ", tmp, rd.addr);
        }
    } else if (end == ':') {
        bool newrt = srcbuf[i + 1] == '\n';
        if (newrt && rtinfo.inrt) {
            endofrt();
            rtinfo.inrt = false;
        }
        srcbuf[i] = '\0';
        char next = srcbuf[i + 1];
        char *label = srcbuf + tok_start;
        if (next == '\n') {
            struct symbol s = {
                .p = label,
                .addr = cur_pc,
                .type = unknwon,
            };
            list_add_symbol_t(&symbols, s);
            printf("\nsym(%s @0x%x) ", s.p, s.addr);
            rst_rtinfo();
            printf("start of routine\n");
            rtinfo.inrt = true;
            rtinfo.loc_symbol_start = symbols.count;
        } else if (IS_LETTER(srcbuf[i + 2])) {
            ++i;
            int tok_start2 = ++i;
            readsym();
            srcbuf[i] = '\0';
            char *next_tok = srcbuf + tok_start2;
            int objsiz = 0;
            bool sign = false;
            if (strcmp(next_tok, "i32") == 0) {
                objsiz = 4;
                sign = true;
            } else if (strcmp(next_tok, "addr") == 0) {
                objsiz = 8;
            }
            struct _object obj = {
                .name = label,
                .offset = rtinfo.stacksiz,
                .size = objsiz,
                .sign = sign,
            };
            rtinfo.stacksiz += objsiz;
            printf("stackobj (%s off %x, siz %x, sign %x) ", label, obj.offset, obj.size, obj.sign);
            list_add_object(&objects, obj);
        }
    } else {
        printf("unknown end (%c)", end);
    }
}

void cmp(int lit) {
    printf("cmp(w%d<-%d) \n", reg, lit);
    OPC(&rtinfo.opc, CMP | (lit << 10));
    cmpmode = false;
}

long readnum(void) {
    size_t tok_start = i;
    while ((c = srcbuf[i])) {
        if (IS_DIGIT(c)) {
            ++i;
            continue;
        }
        break;
    }
    srcbuf[i] = '\0';
    long lit = strtol(srcbuf + tok_start, NULL, 10);
    return lit;
}

void digit(void) {
    size_t tok_start = i;
    while ((c = srcbuf[i])) {
        if (IS_DIGIT(c)) {
            ++i;
            continue;
        }
        srcbuf[i] = '\0';
        long lit = strtol(srcbuf + tok_start, NULL, 10);
        if (cmpmode) {
            cmp(lit);
            break;
        }

        uint32_t op = MOV | lit << 5 | reg;
        printf("mov(w%d<-%ld) \n", reg, lit);
        list_add_uint32_t(&rtinfo.opc, op);
        cur_pc += sizeof (uint32_t);
        break;
    }
}

void check_callsub(void) {
    if (!rtinfo.callsub) {
        /* list_add_uint32_t(&opc_all, STP_PREL); */
        /* cur_pc += sizeof (uint32_t); */
        rtinfo.callsub = true;
    }
}

void ldr_str(bool store) {
    int start_pos = i;
    readsym();
    srcbuf[i - 1] = '\0';
    char *name = srcbuf + start_pos + 1;

    struct _object *pobj = NULL;
    for (int i = 0; i < objects.count; ++i) {
        if (strcmp(name, objects.data[i].name) == 0) {
            pobj = objects.data + i;
        }
    }
    if (pobj == NULL) {
        printf("could not find stack obj.\n");
        return;
    }
    uint16_t offset = pobj->offset;
    uint16_t size = pobj->size;
    bool unscaled = offset % size != 0;
    uint32_t op = 0xb8000000;
    if (!store) {
        op |= 1 << 22;
        op |= reg;
    }
    if (!unscaled)
        op |= 1 << 24;
    if (size == 8)
        op |= 1 << 30;
    else if (size != 4)
        printf("!!!unimpl size!!!");
    char *opname = store ? "str" : "ldr";
    printf("%s obj %s(off %x siz %x) ", opname, name, offset, size);
    if (unscaled) {
        op |= offset << 12;
    } else {
        op |= ((offset / size) << 10);
    }
    op |= (0b11111 << 5);
    OPC(&rtinfo.opc, op);
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
    } else if (next == '[') {
        ldr_str(true);
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
    unsigned long len = strlen(s) + 1;
    size_t nitems = (len) / (sizeof (uint32_t));
    ++nitems;
    printf("string lit (%s, len %lx, nitems %zu) ", s, len, nitems);
    list_addrang_uint32_t(&opc_all, (uint32_t *)s, nitems);
    /* if (strlit_idx + len > strlit_cap) { */
    /*     strlit_cap += 0x4; */
    /*     strlits = reallocf(strlits, strlit_cap); */
    /* } */
    /* strlcpy(strlits + strlit_idx, s, strlit_cap - strlit_idx); */
    cur_pc += nitems * sizeof (uint32_t);
}

void colons(void) {
    c = srcbuf[++i];
    if (c != ':') {
        return;
    }
    c = srcbuf[++i];
    if (c == ' ') {
        c = srcbuf[++i];
    }

    printf("\n(anoyn blk ");
    asprintf(&blk_name, "__anoyn_blk_%d", blk_idx);
    asprintf(&blk_name_brk, "__anoyn_blk_%d.break", blk_idx);
    if (c == ' ') {
        c = srcbuf[++i];
    }
    if (c == '?') {
        printf("cond) ");
        int n = readnum();
        if (n == 0) {
            printf("cbnz ");
            struct _resolve_data tmp = {
                .addr = cur_pc,
                .str = blk_name_brk,
            };
            list_add_resolve_data(&to_resolve, tmp);
            OPC(&rtinfo.opc, CBNZ | reg);
        }
    }
}

void parse(void) {
    for (i = 0; i < filelen - 1; ++i) {
        c = srcbuf[i];
        if (c == '[') {
            ldr_str(false);
        } else if (IS_LETTER(c)) {
            letter();
        } else if (IS_DIGIT(c)) {
            digit();
        } else if (c == '-' || c == '=') {
            hyphen(c == '=');
        } else if (c == '"') {
            strlit_add();
        } else if (c == '/') {
            c = srcbuf[++i];
            if (c == '/') {
                printf("comment \n");
                while (c != '\n')
                    c = srcbuf[++i];
            }
            if (c == '*') {
                printf("multiline comment \n");
rerun_comment:
                while (c != '*')
                    c = srcbuf[++i];
                if (srcbuf[++i] != '/')
                    goto rerun_comment;
            }
        } else if (c == ':') {
            colons();
        } else if (c == '?') {
            cmpmode = true;
        } else if (c == ' ') {
            char before = srcbuf[i - 1];
            if (before != '\0' && before != '\n') {
                printf("before! (%x)\n", before);
                continue;
            }
            int space_count = 0;
            while (c == ' ') {
                c = srcbuf[++i];
                // printf("~");
                ++space_count;
            }
            c = srcbuf[--i];
            if (space_count >= 4) {
                ident_before = ident;
                ident = space_count / 4;
                if (ident_before > ident) {
                    struct symbol s = {
                        .p = blk_name_brk,
                        .addr = cur_pc,
                        .type = code,
                    };
                    list_add_symbol_t(&symbols, s);
                    printf("end of blk, pc: %x\n", s.addr);
                }
                printf("\nident %d(before %d, pc %llx)", ident, ident_before, cur_pc);
            }
        } else if (c > ' ') {
            printf("unknwon tok '%c'(%x) ", c, c);
        }
        handle_reg(c);
    }
}

void resolve_symbols(void) {
    for (int i = 0; i < to_resolve.count; ++i) {
        struct _resolve_data d = to_resolve.data[i];
        printf("try(%s, %x) ", d.str, d.addr);
        struct symbol *s = NULL;
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
            printf("could not find symbol '%s', reloc as undef b\n", d.str);
            struct symbol tmp = {
                .addr = 0,
                .p = d.str,
                .type = code_undef,
            };
            list_add_uint32_t(&relocent, d.addr);
            int idx = symbols.count;
            list_add_uint32_t(&relocent, 0x2d000000 | idx);

            list_add_symbol_t(&symbols, tmp);
            printf("\nundefsym(%s @0x%x) ", tmp.p, tmp.addr);
            ++nundefsyms;
            continue;
        }
        if (s->type == code_undef) {
            list_add_uint32_t(&relocent, d.addr);
            list_add_uint32_t(&relocent, 0x2d000000 | j);
            continue;
        } else if (s->type == unknwon)
            s->type = code;
        size_t idx = d.addr / sizeof (uint32_t);
        uint32_t opcode = opc_all.data[idx];
        if (opcode == B || opcode == BL) {
            uint32_t dif = s->addr - d.addr;
            opc_all.data[idx] |= (dif / sizeof (uint32_t));
            printf("resolv(%s->%x, %x) ", d.str, dif, opc_all.data[idx]);
        } else if ((opcode & 0xfffffc00) == ADD_IMM) {
            list_add_uint32_t(&relocent, d.addr);
            int index = j;
            list_add_uint32_t(&relocent, 0x4c000000 | index);

            // opc_all.data[idx] |= (0xf88) << 10;
            printf("will be resolved by ld(%s, %x) ", d.str, opc_all.data[idx]);
        } else if ((opcode & 0xff000000) == CBNZ) {
            uint32_t dif = s->addr - d.addr;
            opc_all.data[idx] |= (dif / sizeof (uint32_t)) << 5;
            printf("resolv(%s->%x, %x) ", d.str, dif, opc_all.data[idx]);
        } else {
            printf("undefined opcode %x @%zx\n", opc_all.data[idx], idx);
        }
    }
}

int main(void) {
    size_t total_size = 0;
    uint64_t textsect_size = 0;

// #parser
    FILE *src = fopen("main.al", "r");
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

    list_new_object(&objects, INIT_CAP, "obj");
    list_new_uint32_t(&relocent, INIT_CAP, "reloc");
    list_new_uint32_t(&opc_all, 0x100, "opc");
    list_new_uint32_t(&rtinfo.opc, INIT_CAP, "rt");
    list_new_resolve_data(&to_resolve, INIT_CAP, "resolv");
    list_new_symbol_t(&symbols, INIT_CAP, "symb");
    strlits = malloc(INIT_CAP);

    printf(":: parse start\n");
    parse();
    endofrt();
    printf("\n:: parse end\n");
    resolve_symbols();
    printf("\n:: resolv end\n");

    struct list_uint64_t symtab_data;
    list_new_uint64_t(&symtab_data, INIT_CAP, "symtab");

    int strtab_size = 0x10;
    char *strtab = malloc(strtab_size * sizeof (char));
    strtab[0] = '\0';
    int strtab_idx = 1;

    for (int i = 0; i < symbols.count; ++i) {
        struct symbol data = symbols.data[i];
        char *s = data.p;
        unsigned long len = strlen(s);
        // printf("%s\n", s);
        while (strtab_idx + len >= strtab_size) {
            strtab_size += 0x10;
            strtab = reallocf(strtab, strtab_size);
        }
        strlcpy(strtab + strtab_idx, s, strtab_size - strtab_idx);
        uint64_t flag;
        if (data.type == code_undef) {
            flag = 0x0100000000;
        } else {
            flag = 0x010f00000000;
        }
        list_add_uint64_t(&symtab_data, flag | strtab_idx);
        list_add_uint64_t(&symtab_data, data.addr);
        strtab_idx += strlen(s) + 1;
    }
    printf("done strtab\n");

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
    printf("textsect (%llx), opcsiz (%x)\n", textsect_size, opc_size);


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
    printf("strtab (%x)\n", strtab_size);

// # lc dysymtab
    struct dysymtab_command dysymtab;
    memset(&dysymtab, 0, sizeof dysymtab);
    dysymtab.cmd = LC_DYSYMTAB;
    dysymtab.cmdsize = sizeof dysymtab;
    LC_REG(dysymtab);

    dysymtab.ilocalsym = 0x0;
    dysymtab.nlocalsym = symbols.count - nundefsyms;
    dysymtab.iextdefsym = dysymtab.ilocalsym + dysymtab.nlocalsym;
    dysymtab.nextdefsym = 0x0;

    dysymtab.iundefsym = dysymtab.iextdefsym + dysymtab.nextdefsym;
    dysymtab.nundefsym = nundefsyms;

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
    unsigned long relocent_size = relocent.count * sizeof relocent.data[0];
    total_size += relocent_size;

    sect.offset = segload.fileoff = sizeof header + header.sizeofcmds;
    sect.reloff = sect.offset + textsect_size;
    sect.nreloc = relocent.count / 2;
    symtab.symoff = sect.reloff + relocent_size;
    symtab.stroff = symtab.symoff + symtab_data_size;
    total_size += load_cmds_size;

// buffer

    void *buf = malloc(total_size);
    void *buf_base = buf;
    printf("total (%zx)\n", total_size);

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
    /* memcpy(buf, strlits, strlit_cap); */
    /* buf += strlit_cap; */
    // add reloc entries
    memcpy(buf, relocent.data, relocent_size);
    buf += relocent_size;
    // add symtab data
    memcpy(buf, symtab_data.data, symtab_data_size);
    buf += symtab_data_size;
    // add strtab
    memcpy(buf, strtab, strtab_size);
    buf += strtab_size;

    FILE *file = fopen("machoreloc.out", "w");
    fwrite(buf_base, sizeof (char), total_size, file);
    fclose(file);
    // not freed on purpose for performance gain
    /* free(buf_base); */
    /* list_delete_symbol_t(&symbols); */
    /* list_delete_object(&relocent); */
    /* list_delete_uint32_t(&relocent); */
    /* list_delete_uint32_t(&opc_all); */
    /* list_delete_resolve_data(&to_resolve); */
    /* list_delete_uint64_t(&symtab_data); */
}

