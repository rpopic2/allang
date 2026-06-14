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

static void write_macho(FILE *out) {
    _Alignas(16) static uint8_t img[0x8000];

    bin_image image;
    bin_emit(&image);

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
    sect.size = image.text_size;
    sect.align = 2;
    sect.flags = S_ATTR_PURE_INSTRUCTIONS | S_ATTR_SOME_INSTRUCTIONS;
    struct section_64 *psect = add_payload(&b, &sect, sizeof sect);

    struct segment_command_64 linkedit = {0};
    linkedit.cmd = LC_SEGMENT_64;
    linkedit.cmdsize = (uint32_t)sizeof linkedit;
    set_name(linkedit.segname, SEG_LINKEDIT);
    linkedit.vmaddr = TEXT_VM + FILE_PAGE;
    linkedit.vmsize = FILE_PAGE;
    linkedit.fileoff = FILE_PAGE;
    linkedit.maxprot = VM_PROT_READ;
    linkedit.initprot = VM_PROT_READ;
    struct segment_command_64 *plinkedit = add_segment(&b, &linkedit, sizeof linkedit);

    uint32_t cf_starts_off = align_up((uint32_t)sizeof(struct dyld_chained_fixups_header), 8u);
    uint32_t cf_size = cf_starts_off + (uint32_t)(sizeof(uint32_t) * (1u + b.nsegs));

    uint32_t le = FILE_PAGE;
    uint32_t cf_off = le_reserve(&le, cf_size);
    le = align_up(le, 16u);
    uint32_t sig_off = le;
    uint32_t code_limit = sig_off;
    uint32_t n_slots = (code_limit + CS_PAGE - 1u) / CS_PAGE;
    uint32_t ident_len = (uint32_t)strlen(CD_IDENT) + 1u;
    uint32_t sb_len = 20u + (CD_HDR_SZ + ident_len) + n_slots * CC_SHA256_DIGEST_LENGTH;
    le_reserve(&le, sb_len);
    plinkedit->filesize = le - FILE_PAGE;

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

    b.off = code_off;
    bld_append(&b, image.text, image.text_size);

    struct dyld_chained_fixups_header cfh = {0};
    cfh.starts_offset = cf_starts_off;
    cfh.imports_offset = cf_size;
    cfh.symbols_offset = cf_size;
    cfh.imports_format = DYLD_CHAINED_IMPORT;
    memcpy(img + cf_off, &cfh, sizeof cfh);
    struct dyld_chained_starts_in_image starts = {0};
    starts.seg_count = b.nsegs;
    memcpy(img + cf_off + cf_starts_off, &starts, sizeof starts);

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
void emit_text(FILE *out) {}
void emit_cstr(FILE *out) {
    write_macho(out);
}

#pragma clang diagnostic pop

const char *text_section_header = "";
const char *string_section_header = "";
const char *output_ext = "";
