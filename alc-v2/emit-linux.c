#include "emit.h"

#define INSTR(s) "\t"s"\n"

const char *text_section_header = INSTR(".text");
const char *string_section_header = INSTR(".section	.rodata.str1.1,\"aMS\",@progbits,1");

const char *addrgen_adrp = INSTR("adrp x%d, %s");
const char *addrgen_add = INSTR("add x%d, x%d, :lo12:%s");
const char *fn_prefix = "";
const char *mainfn_annotation = ".type main, @function";
const char *local_string_prefix = ".L.str.%d";
