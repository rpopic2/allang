const char *text_section_header = ("\t.text\n");
const char *string_section_header = ("");

// x64 abi

const char * const rname_scratch[] = {
	"ax", "cx", "dx", "r8", "r9", "r10", "r11"
};
const size_t rname_scratch_len = sizeof rname_scratch / sizeof (char *);

const char * const rname_callee[] = {
	"bx", "si", "di", "r12", "r13", "r14", "r15"
};
const size_t rname_callee_len = sizeof rname_callee / sizeof (char *);

const char * const rname_param[] = {
	"cx", "dx", "r8", "r9", // end of x64
	"r10", "r11",
};
const size_t rname_param_len = sizeof rname_param / sizeof (char *);

const char * const rname_ret[] = {
	"ax",	// end of x64
	"dx"
};
const size_t rname_ret_len = sizeof rname_ret / sizeof (char *);
