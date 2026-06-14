#include <stdio.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <CommonCrypto/CommonDigest.h>
#include <mach/machine.h>
#include <mach/vm_prot.h>
#include <mach-o/loader.h>
#include <mach-o/fixup-chains.h>

#include "emit.h"
#include "emit-bin.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

#define TEXT_VM   0x100000000ULL
#define FILE_PAGE 0x4000u
#define CS_PAGE   4096u

#define DYLD_PATH "/usr/lib/dyld"
#define LIB_PATH  "/usr/lib/libSystem.B.dylib"
#define DATA_VM   (TEXT_VM + FILE_PAGE)

#define CD_HDR_SZ 88u
#define CD_IDENT  "alc"

static void put_u32_be(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)((v >> 24) & 0xffu);
    p[1] = (uint8_t)((v >> 16) & 0xffu);
    p[2] = (uint8_t)((v >> 8) & 0xffu);
    p[3] = (uint8_t)(v & 0xffu);
}

static void put_u64_be(uint8_t *p, uint64_t v) {
    put_u32_be(p, (uint32_t)(v >> 32));
    put_u32_be(p + 4, (uint32_t)(v & 0xffffffffu));
}

typedef struct {
    uint8_t *img;
    size_t off;
    uint32_t ncmds;
    uint32_t sizeofcmds;
    uint32_t nsegs;
} builder;

static void *bld_append(builder *b, const void *p, size_t n) {
    void *dst = b->img + b->off;
    memcpy(dst, p, n);
    b->off += n;
    return dst;
}

static void *add_command(builder *b, const void *cmd, size_t n) {
    void *p = bld_append(b, cmd, n);
    b->ncmds += 1;
    b->sizeofcmds += (uint32_t)n;
    return p;
}

static void *add_payload(builder *b, const void *p, size_t n) {
    void *dst = bld_append(b, p, n);
    b->sizeofcmds += (uint32_t)n;
    return dst;
}

static void *add_segment(builder *b, const void *cmd, size_t n) {
    void *p = add_command(b, cmd, n);
    b->nsegs += 1;
    return p;
}

static uint32_t align_up(uint32_t x, uint32_t a) {
    return (x + a - 1u) & ~(a - 1u);
}

static uint32_t le_reserve(uint32_t *cursor, uint32_t size) {
    uint32_t off = *cursor;
    *cursor += size;
    return off;
}

static void set_name(char dst[16], const char *s) {
    memcpy(dst, s, strlen(s));
}

static void write_signature(uint8_t *sig, uint32_t code_limit, uint32_t n_slots, const uint8_t hashes[][CC_SHA256_DIGEST_LENGTH]) {
    uint32_t ident_len = (uint32_t)strlen(CD_IDENT) + 1;
    uint32_t cd_hash_off = CD_HDR_SZ + ident_len;
    uint32_t cd_len = cd_hash_off + n_slots * CC_SHA256_DIGEST_LENGTH;
    uint32_t sb_len = 20u + cd_len;

    put_u32_be(sig + 0, 0xfade0cc0u);
    put_u32_be(sig + 4, sb_len);
    put_u32_be(sig + 8, 1u);
    put_u32_be(sig + 12, 0u);
    put_u32_be(sig + 16, 20u);

    uint8_t *cd = sig + 20;
    put_u32_be(cd + 0, 0xfade0c02u);
    put_u32_be(cd + 4, cd_len);
    put_u32_be(cd + 8, 0x00020400u);
    put_u32_be(cd + 12, 0x00000002u);
    put_u32_be(cd + 16, cd_hash_off);
    put_u32_be(cd + 20, CD_HDR_SZ);
    put_u32_be(cd + 24, 0u);
    put_u32_be(cd + 28, n_slots);
    put_u32_be(cd + 32, code_limit);
    cd[36] = CC_SHA256_DIGEST_LENGTH;
    cd[37] = 2;
    cd[38] = 0;
    cd[39] = 12;
    put_u32_be(cd + 40, 0u);
    put_u32_be(cd + 44, 0u);
    put_u32_be(cd + 48, 0u);
    put_u32_be(cd + 52, 0u);
    put_u64_be(cd + 56, 0u);
    put_u64_be(cd + 64, 0u);
    put_u64_be(cd + 72, FILE_PAGE);
    put_u64_be(cd + 80, 1u);

    memcpy(cd + CD_HDR_SZ, CD_IDENT, ident_len);
    for (uint32_t i = 0; i < n_slots; i++)
        memcpy(cd + cd_hash_off + i * CC_SHA256_DIGEST_LENGTH, hashes[i], CC_SHA256_DIGEST_LENGTH);
}

static uint32_t build_chained_imports(uint8_t *dst, uint32_t nsegs, uint32_t data_seg_idx,
                                      uint32_t n_imp, const bin_import *imports) {
    struct dyld_chained_fixups_header *h = (struct dyld_chained_fixups_header *)dst;
    memset(h, 0, sizeof *h);
    uint32_t sii_off = align_up((uint32_t)sizeof *h, 8u);
    h->starts_offset = sii_off;
    h->imports_count = n_imp;
    h->imports_format = DYLD_CHAINED_IMPORT;

    uint8_t *sii = dst + sii_off;
    memcpy(sii, &nsegs, 4);
    memset(sii + 4, 0, 4u * nsegs);
    uint32_t sis_off = align_up(sii_off + 4u + 4u * nsegs, 8u);
    uint32_t rel = sis_off - sii_off;
    memcpy(sii + 4 + 4 * data_seg_idx, &rel, 4);

    struct dyld_chained_starts_in_segment *sis =
        (struct dyld_chained_starts_in_segment *)(dst + sis_off);
    memset(sis, 0, sizeof *sis);
    sis->size = (uint32_t)sizeof *sis;
    sis->page_size = FILE_PAGE;
    sis->pointer_format = DYLD_CHAINED_PTR_64_OFFSET;
    sis->segment_offset = FILE_PAGE;
    sis->page_count = 1;
    sis->page_start[0] = 0;

    uint32_t imports_off = align_up(sis_off + (uint32_t)sizeof *sis, 4u);
    h->imports_offset = imports_off;
    uint32_t symbols_off = align_up(imports_off + 4u * n_imp, 4u);
    h->symbols_offset = symbols_off;

    uint32_t s = symbols_off;
    dst[s++] = '\0';
    for (uint32_t i = 0; i < n_imp; i++) {
        uint32_t name_off = s - symbols_off;
        dst[s++] = '_';
        size_t len = strlen(imports[i].name);
        memcpy(dst + s, imports[i].name, len);
        s += (uint32_t)len;
        dst[s++] = '\0';
        uint32_t entry = 1u | (name_off << 9);
        memcpy(dst + imports_off + 4u * i, &entry, 4);
    }
    return s;
}

static void write_macho(FILE *out) {
    _Alignas(16) static uint8_t img[0x10000];

    bin_image image;
    bin_emit(&image);
    uint32_t n_imp = image.imports_count;
    bool dyn = n_imp > 0;
    uint32_t nsegs = dyn ? 4u : 3u;
    uint32_t data_seg_idx = 2u;

    uint32_t le_fileoff = dyn ? 2u * FILE_PAGE : FILE_PAGE;
    uint32_t cf_off = le_fileoff;
    uint32_t cf_size;
    if (dyn) {
        cf_size = build_chained_imports(img + cf_off, nsegs, data_seg_idx, n_imp, image.imports);
    } else {
        uint32_t starts_off = align_up((uint32_t)sizeof(struct dyld_chained_fixups_header), 8u);
        struct dyld_chained_fixups_header cfh = {0};
        cfh.starts_offset = starts_off;
        cfh.imports_format = DYLD_CHAINED_IMPORT;
        struct dyld_chained_starts_in_image starts = {0};
        starts.seg_count = nsegs;
        cf_size = starts_off + (uint32_t)(sizeof(uint32_t) * (1u + nsegs));
        cfh.imports_offset = cf_size;
        cfh.symbols_offset = cf_size;
        memcpy(img + cf_off, &cfh, sizeof cfh);
        memcpy(img + cf_off + starts_off, &starts, sizeof starts);
    }

    uint32_t sig_off = align_up(cf_off + cf_size, 16u);
    uint32_t code_limit = sig_off;
    uint32_t n_slots = (code_limit + CS_PAGE - 1u) / CS_PAGE;
    uint32_t ident_len = (uint32_t)strlen(CD_IDENT) + 1u;
    uint32_t sb_len = 20u + (CD_HDR_SZ + ident_len) + n_slots * CC_SHA256_DIGEST_LENGTH;

    uint32_t text_bytes = (uint32_t)image.text_size;
    uint32_t stub_pad = align_up(text_bytes, 4u) - text_bytes;
    uint32_t sect_size = text_bytes + stub_pad + n_imp * 12u;

    builder b = { .img = img, .off = sizeof(struct mach_header_64) };

    struct segment_command_64 pagezero = {0};
    pagezero.cmd = LC_SEGMENT_64;
    pagezero.cmdsize = (uint32_t)sizeof pagezero;
    set_name(pagezero.segname, SEG_PAGEZERO);
    pagezero.vmsize = TEXT_VM;
    add_segment(&b, &pagezero, sizeof pagezero);

    struct segment_command_64 text = {0};
    text.cmd = LC_SEGMENT_64;
    text.cmdsize = (uint32_t)(sizeof text + sizeof(struct section_64));
    set_name(text.segname, SEG_TEXT);
    text.vmaddr = TEXT_VM;
    text.vmsize = FILE_PAGE;
    text.filesize = FILE_PAGE;
    text.maxprot = VM_PROT_READ | VM_PROT_EXECUTE;
    text.initprot = VM_PROT_READ | VM_PROT_EXECUTE;
    text.nsects = 1;
    add_segment(&b, &text, sizeof text);

    struct section_64 sect = {0};
    set_name(sect.sectname, SECT_TEXT);
    set_name(sect.segname, SEG_TEXT);
    sect.size = sect_size;
    sect.align = 2;
    sect.flags = S_ATTR_PURE_INSTRUCTIONS | S_ATTR_SOME_INSTRUCTIONS;
    struct section_64 *psect = add_payload(&b, &sect, sizeof sect);

    struct section_64 *pgot = NULL;
    if (dyn) {
        struct segment_command_64 data = {0};
        data.cmd = LC_SEGMENT_64;
        data.cmdsize = (uint32_t)(sizeof data + sizeof(struct section_64));
        set_name(data.segname, "__DATA_CONST");
        data.vmaddr = DATA_VM;
        data.vmsize = FILE_PAGE;
        data.fileoff = FILE_PAGE;
        data.filesize = FILE_PAGE;
        data.maxprot = VM_PROT_READ | VM_PROT_WRITE;
        data.initprot = VM_PROT_READ | VM_PROT_WRITE;
        data.nsects = 1;
        add_segment(&b, &data, sizeof data);

        struct section_64 got = {0};
        set_name(got.sectname, "__got");
        set_name(got.segname, "__DATA_CONST");
        got.addr = DATA_VM;
        got.size = n_imp * 8u;
        got.offset = FILE_PAGE;
        got.align = 3;
        pgot = add_payload(&b, &got, sizeof got);
    }

    struct segment_command_64 linkedit = {0};
    linkedit.cmd = LC_SEGMENT_64;
    linkedit.cmdsize = (uint32_t)sizeof linkedit;
    set_name(linkedit.segname, SEG_LINKEDIT);
    linkedit.vmaddr = TEXT_VM + le_fileoff;
    linkedit.vmsize = FILE_PAGE;
    linkedit.fileoff = le_fileoff;
    linkedit.filesize = (sig_off + sb_len) - le_fileoff;
    linkedit.maxprot = VM_PROT_READ;
    linkedit.initprot = VM_PROT_READ;
    add_segment(&b, &linkedit, sizeof linkedit);

    struct linkedit_data_command chained = {0};
    chained.cmd = LC_DYLD_CHAINED_FIXUPS;
    chained.cmdsize = (uint32_t)sizeof chained;
    chained.dataoff = cf_off;
    chained.datasize = cf_size;
    add_command(&b, &chained, sizeof chained);

    uint32_t dyld_cmdsize = align_up((uint32_t)(sizeof(struct dylinker_command) + strlen(DYLD_PATH) + 1u), 8u);
    struct dylinker_command dylinker = {0};
    dylinker.cmd = LC_LOAD_DYLINKER;
    dylinker.cmdsize = dyld_cmdsize;
    dylinker.name.offset = (uint32_t)sizeof dylinker;
    add_command(&b, &dylinker, sizeof dylinker);
    char dyld_path[sizeof(DYLD_PATH) + 8] = {0};
    memcpy(dyld_path, DYLD_PATH, strlen(DYLD_PATH));
    add_payload(&b, dyld_path, dyld_cmdsize - sizeof(struct dylinker_command));

    if (dyn) {
        uint32_t lib_cmdsize = align_up((uint32_t)(sizeof(struct dylib_command) + strlen(LIB_PATH) + 1u), 8u);
        struct dylib_command lib = {0};
        lib.cmd = LC_LOAD_DYLIB;
        lib.cmdsize = lib_cmdsize;
        lib.dylib.name.offset = (uint32_t)sizeof lib;
        lib.dylib.timestamp = 2;
        lib.dylib.current_version = 0x05470000u;
        lib.dylib.compatibility_version = 0x00010000u;
        add_command(&b, &lib, sizeof lib);
        char lib_path[sizeof(LIB_PATH) + 8] = {0};
        memcpy(lib_path, LIB_PATH, strlen(LIB_PATH));
        add_payload(&b, lib_path, lib_cmdsize - sizeof(struct dylib_command));
    }

    struct build_version_command build = {0};
    build.cmd = LC_BUILD_VERSION;
    build.cmdsize = (uint32_t)sizeof build;
    build.platform = PLATFORM_MACOS;
    build.minos = 0x000d0000u;
    build.sdk = 0x000d0000u;
    add_command(&b, &build, sizeof build);

    struct entry_point_command main_cmd = {0};
    main_cmd.cmd = LC_MAIN;
    main_cmd.cmdsize = (uint32_t)sizeof main_cmd;
    struct entry_point_command *pmain = add_command(&b, &main_cmd, sizeof main_cmd);

    struct linkedit_data_command codesig = {0};
    codesig.cmd = LC_CODE_SIGNATURE;
    codesig.cmdsize = (uint32_t)sizeof codesig;
    codesig.dataoff = sig_off;
    codesig.datasize = sb_len;
    add_command(&b, &codesig, sizeof codesig);

    uint32_t code_off = (uint32_t)b.off;
    psect->addr = TEXT_VM + code_off;
    psect->offset = code_off;
    pmain->entryoff = code_off + image.entry;

    struct mach_header_64 hdr = {0};
    hdr.magic = MH_MAGIC_64;
    hdr.cputype = CPU_TYPE_ARM64;
    hdr.cpusubtype = CPU_SUBTYPE_ARM64_ALL;
    hdr.filetype = MH_EXECUTE;
    hdr.ncmds = b.ncmds;
    hdr.sizeofcmds = b.sizeofcmds;
    hdr.flags = MH_NOUNDEFS | MH_DYLDLINK | MH_TWOLEVEL | MH_PIE;
    memcpy(img, &hdr, sizeof hdr);

    memcpy(img + code_off, image.text, text_bytes);

    uint32_t stub_off = code_off + text_bytes + stub_pad;
    for (uint32_t i = 0; i < n_imp; i++) {
        uint64_t stub_vm = TEXT_VM + stub_off + i * 12u;
        uint64_t got_vm = DATA_VM + i * 8u;
        int64_t pages = (int64_t)((got_vm & ~0xfffULL) - (stub_vm & ~0xfffULL)) >> 12;
        uint32_t immlo = (uint32_t)(pages & 3);
        uint32_t immhi = (uint32_t)(pages >> 2) & 0x7ffffu;
        uint32_t adrp = 0x90000010u | (immlo << 29) | (immhi << 5);
        uint32_t ldr = 0xf9400210u | (((uint32_t)(got_vm & 0xfffu) >> 3) << 10);
        uint32_t br = 0xd61f0200u;
        memcpy(img + stub_off + i * 12u + 0, &adrp, 4);
        memcpy(img + stub_off + i * 12u + 4, &ldr, 4);
        memcpy(img + stub_off + i * 12u + 8, &br, 4);
    }

    for (uint32_t i = 0; i < image.extcalls_count; i++) {
        const bin_extcall *e = &image.extcalls[i];
        uint64_t bl_vm = TEXT_VM + code_off + e->site;
        uint64_t stub_vm = TEXT_VM + stub_off + e->import * 12u;
        int32_t rel = (int32_t)((int64_t)(stub_vm - bl_vm) >> 2);
        uint32_t bl = 0x94000000u | ((uint32_t)rel & 0x3ffffffu);
        memcpy(img + code_off + e->site, &bl, 4);
    }

    if (dyn) {
        for (uint32_t i = 0; i < n_imp; i++) {
            uint64_t next = (i + 1u < n_imp) ? 2u : 0u;
            uint64_t slot = (1ull << 63) | (next << 51) | (uint64_t)i;
            memcpy(img + FILE_PAGE + i * 8u, &slot, 8);
        }
        pgot->reserved1 = 0;
    }

    uint8_t hashes[sizeof img / CS_PAGE + 1u][CC_SHA256_DIGEST_LENGTH];
    for (uint32_t i = 0; i < n_slots; i++) {
        uint32_t start = i * CS_PAGE;
        uint32_t len = (code_limit - start > CS_PAGE) ? CS_PAGE : (code_limit - start);
        CC_SHA256(img + start, len, hashes[i]);
    }

    write_signature(img + sig_off, code_limit, n_slots, hashes);

    fwrite(img, 1, sig_off + sb_len, out);
    fflush(out);
    fchmod(fileno(out), 0755);
}

void emit_init(void) {}
void emit_output(FILE *out) {
    write_macho(out);
}

#pragma clang diagnostic pop

const char *text_section_header = "";
const char *string_section_header = "";
const char *output_ext = "";
