#pragma once

#include "emit.h"
void compile_err(const token_t *token, const char *format, ...);
void compile_warning(const char *format, ...);

#if NDEBUG
#define printd(...) ((void)0)
#define str_printd(...) ((void)0)
#define str_printdnl(S) ((void)0)

#else
#define printd(...) printf(__VA_ARGS__)
#define str_printd(...) str_print(__VA_ARGS__)
#define str_printdnl(S) str_fprintnl(S, stdout)
#endif

#if DEBUG_TIMER
#define TIMER_START(name) clock_t name = clock();
#define TIMER_END(name) clock_t name##_time = (clock() - name); \
    printf(#name": %.3lfms(%luμs) elapsed\n", name##_time / ((double)CLOCKS_PER_SEC / 1000), name##_time);
#define TIMER_LABEL(s) printf(s);
#define TIMER_LABEL_STR(str) printf(CSI_GREEN"\nlabel "), str_print(str), printf("-----\n"CSI_RESET);

#else

#define TIMER_START(name) ((void)0)
#define TIMER_END(name) ((void)0)
#define TIMER_LABEL(s) ((void)0)
#define TIMER_LABEL_STR(str) ((void)0)

#endif
