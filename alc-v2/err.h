#pragma once

typedef struct token token_t;
void compile_err(const token_t *token, const char *format, ...);

#if NDEBUG
#define printd(...) ((void)0)
#define print(...) ((void)0)
#define str_printd(...) ((void)0)
#define str_printdnl(S) ((void)0)
// p* series are intended to be compiler error here, as they are only for inspection

#else
#define printd(...) printf(__VA_ARGS__)
#define print(category, ...) if (category) printf(__VA_ARGS__)
#define str_printd(...) str_print(__VA_ARGS__)
#define str_printdnl(S) str_fprintnl(S, stdout)
#define pf(...) printf(__VA_ARGS__), putc('\n', stdout);
#define p(s) printf(#s), putc('\n', stdout);
#define pi(i) printf(#i": %lld\n", (long long)(i));
#define pp(i) printf(#i": %p\n", (void *)(i));
#define pc(i) printf(#i": '%c'\n", i);
#define ps(s) printf(#s": "), str_print(s);
#endif

#if DEBUG_TIMER
#define TIMER_START(name) clock_t name = clock();
#define TIMER_END(name) clock_t name##_time = (clock() - name); \
    printf(#name": %.3lfms(%luμs) elapsed\n", (double)name##_time / ((double)CLOCKS_PER_SEC / 1000), name##_time);
#define TIMER_LABEL(s) printf(s);
#define TIMER_LABEL_STR(str) printf(CSI_GREEN"\nlabel "), str_print(str), printf("-----\n"CSI_RESET);

#else

#define TIMER_START(name) ((void)0)
#define TIMER_END(name) ((void)0)
#define TIMER_LABEL(s) ((void)0)
#define TIMER_LABEL_STR(str) ((void)0)

#endif
