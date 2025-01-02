#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *MACHO_HEADER =  // magic            // cpu type
                            "\xcf\xfa\xed\xfe" "\x0c\x00\x00\x01"
                            // cpu subtype      // filetype (1=reloc, 2=exe)
                            "\x00\x00\x00\x00" "\x01\x00\x00\x00"
                            // num load cmds    //
                            "\x04\x00\x00\x00" 
                            ;



typedef uint32_t u32;
typedef uint64_t u64;

enum load_cmd_type {
    lc_linkedit_symtab = 0x2,
    lc_linkedit_symtab_info = 0xb,
    lc_64bit_seg_load = 0x19,
    lc_minimum_os_ver = 0x32,
};
struct load_cmd {
    enum load_cmd_type type;
    u32 cmd_size;
    char *seg_name;
    u64 addr;
    u64 addr_size;
    u64 file_off;
    u64 size;
    u32 max_vm_protection;
    u32 init_vm_protection;
};

void x(u32 *p, int *idx, u64 x) {
    u64 *q = (u64 *)(p + *idx);
    *q = x;
    *idx += 2;
}

void s(u32 *p, int *idx, const char *val) {
    if (!val) {
        x(p, idx, 0);
        return;
    }
    memcpy(p + *idx, val, sizeof(u64));
    *idx += 2;
}

int main(void) {
    char *buffer = calloc(0x400, sizeof(char));

    FILE *file = fopen("src.al", "r");
    unsigned long read = fread(buffer, sizeof(char), 0x400, file);
    fclose(file);

    printf("%sEOF\n", buffer);

    u32 *out = calloc(0x100, sizeof(int));
    int idx = 0;

#define W(val) out[idx++] = (val) ;
#define X(val) x(out, &idx, val);
    const u32 magic = 0xfeedfacf;
    const u32 cpu_type_arm_64bit = 0x0100000c;
    const u32 cpu_subtype_arm_all = 0x0;
    enum filetype : u32 {
        ft_reloc = 1, ft_exe = 2
    };
    // header
    W(magic) W(cpu_type_arm_64bit) W(cpu_subtype_arm_all) W(ft_reloc)
    u32 num_load_cmds = 0x4;
    u32 size_load_cmds = 0x118;
    const u32 reserved = 0x0;
    W(num_load_cmds) W(size_load_cmds) W(reserved) W(reserved)

    // load cmds

    u32 cmd_size = 0x98;
#define S(val) s(out, &idx, val);
    W(lc_64bit_seg_load) W(cmd_size) S(NULL)
                        // addr to write sect
    S(NULL) X(0)

    u64 addr_size = 0x4;
    u64 file_off = 0x138;
    X(addr_size) X(file_off)
    u32 size_from_file_off = 0x4;
    X(size_from_file_off) W(0x07) W(0x07)
    u32 num_sect = 0x1;
    W(num_sect) W(0) S("__text\0\0")
    X(0) S("__TEXT\0\0")
    X(0) X(0)
    u32 sect_size = 0x4;
    X(sect_size) W(0x138) W(0x2)
    u32 reloc_file_off = 0x0;
    u32 num_of_relocs = 0x0;
    W(reloc_file_off) W(num_of_relocs) W(0x80000400) W(reserved)
    X(reserved)

                    W(lc_minimum_os_ver) W(0x18)
    enum platform_type {
        pt_macos = 1,
    };
    W(pt_macos) W(0x000e0000) W(0) W(0)

    u32 symbols_num = 0x2;
    u32 symbols_off = 0x140;
    W(lc_linkedit_symtab) W(0x18) W(symbols_off) W(symbols_num)
    u32 strtab_off_rel_header = 0x160;
    u32 strtab_size = 0x10;
    W(strtab_off_rel_header) W(strtab_size)

                         W(lc_linkedit_symtab_info) W(0x50)
    u32 local_sym_idx = 0;
    u32 local_sym_num = 1;
    u32 ext_sym_idx = local_sym_idx + local_sym_num;
    u32 ext_sym_num = 1;
    W(local_sym_idx) W(local_sym_num) W(ext_sym_idx) W(ext_sym_num)

    u32 undef_sym_idx = ext_sym_idx + ext_sym_num;
    u32 undef_sym_num = 0;
    W(undef_sym_idx) W(undef_sym_num) W(0) W(0)
    W(0) W(0) W(0) W(0)
    W(0) W(0) W(0) W(0)
    W(0) W(0)

    // W(0xd2800000)   // mov w0, 0
                W(0xd65f03c0) W(0x0)  // ret

    X(0x010e00000007) X(0)
    X(0x010f00000001) X(0)
    S("\0_main\0l") S("tmp0\0\0\0\0")

    FILE *ofile = fopen("a.out", "w");
    fwrite(out, sizeof(u32), idx, ofile);
    fclose(ofile);

    free(buffer);
}

