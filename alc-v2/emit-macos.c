#include "emit.h"

#define INSTR(s) "\t"s"\n"

const char *text_section_header = INSTR(".section	__TEXT,__text,regular,pure_instructions");
const char *string_section_header = INSTR(".section	__TEXT,__cstring,cstring_literals");
const char *addrgen_adrp = INSTR("adrp x%d, %s@PAGE");
const char *addrgen_add = INSTR("add x%d, x%d, %s@PAGEOFF");
const char *fn_prefix = "_";
const char *mainfn_annotation = "";
const char *local_string_prefix = "l_.str.%d";
