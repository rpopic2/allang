#include "emit.h"

#define INSTR(s) "\t"s"\n"

const char *text_section_header = INSTR(".text");
const char *string_section_header = INSTR(".section	.rodata.str1.1,\"aMS\",@progbits,1");

const char *addrgen_adrp = INSTR("adrp x%d, %s");
const char *addrgen_add = INSTR("add x%d, x%d, :lo12:%s");
const char *fn_prefix = "";
const char *mainfn_annotation = INSTR(".type main, @function");
const char *local_string_prefix = ".L.str.%d";

// system-v abi

const char * const rname_scratch[] = {
	"ax", "cx", "dx", "si", "di", "r8", "r9", "r10", "r11"
};
const size_t rname_scratch_len = sizeof rname_scratch / sizeof (char *);

const char * const rname_callee[] = {
	"bx", "r12", "r13", "r14", "r15"
};
const size_t rname_callee_len = sizeof rname_callee / sizeof (char *);

const char * const rname_param[] = {
	"di", "si", "dx", "cx", "r8", "r9", // end of abi
	"r10", "r11",
};
const size_t rname_param_len = sizeof rname_param / sizeof (char *);

const char * const rname_ret[] = {
	"ax", "dx"
};
const size_t rname_ret_len = sizeof rname_ret / sizeof (char *);

