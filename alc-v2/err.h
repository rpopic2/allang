#pragma once

#include "emit.h"
void compile_err(const token_t *token, const char *format, ...);
void compile_warning(const char *format, ...);

inline void dummy(void) {}
#define DEBUG 0

#if DEBUG
#define printd(...) printf(__VA_ARGS__)
#define str_printd(...) str_print(__VA_ARGS__)
#define str_printdnl(S) str_fprintnl(S, stdout)
#else
#define printd(...) dummy()
#define str_printd(...) dummy()
#define str_printdnl(S) dummy()
#endif

