#pragma once

#include "types.h"

#define DIAGRAM_SCALE_AUTO 0

void compile_err(const token_t *token, const char *format, ...);
void compile_warning(const token_t *token, const char *format, ...);
str dtype_to_str(const dtype_t *self, allocator *alloc);
void diagnostic_slice(const token_t *token, i64 begin_index, i64 end_index, i32 array);
bool diagnostic_dyn_elem_access(const parser_context *context, const regable *offset_regable);

void str_printerr(str s);
void str_printerrnl(str s);
void puterr(const char *s);

void struct_diagram(type_t *type, long scale);
void stack_diagram(parser_context *context, long scale);
void struct_report(type_t *type);
void stack_report(parser_context *context);
void struct_expr_report(dyn_agg_member *args, type_t *type, int depth);

#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 1, 2)))
#endif
    void report_err(const char *format, ...);

#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 1, 2)))
#endif
    void report_backtrace(const char *format, ...);

#if NDEBUG
    #define printd(...) ((void)0)
    #define print(...) ((void)0)
    #define str_printd(...) ((void)0)
    #define str_printdnl(S) ((void)0)

#else
    #define printd(...) printf(__VA_ARGS__)
    #define print(category, ...) if (category) printf(__VA_ARGS__)
    #define str_printd(...) str_print(__VA_ARGS__)
    #define str_printdnl(S) str_fprintnl(S, stdout)
#endif

#ifndef NDEBUG
    // p* series are intended to be compiler error when NDEBUG, as they are only for inspection
    #define pf(...) fprintf(stderr __VA_ARGS__), putc('\n', stderr);
    #define p(s) fprintf(stderr, #s), putc('\n', stderr);
    #define pe(s) fputs(#s, stderr), putc('\n', stderr);
    #define pd(i) fprintf(stderr, #i": %lld\n", (long long)(i));
    #define px(i) fprintf(stderr, #i": %llx\n", (long long)(i));
    #define pp(i) fprintf(stderr, #i": %p\n", (void *)(i));
    #define pc(i) fprintf(stderr, #i": '%c'\n", i);
    #define pcs(s) fprintf(sderr, #s": '%s'\n", s);
    #define ps(s) fprintf(stderr, #s": "), str_printerr(s);
    #define bt report_backtrace("\n");
    #define pdtype(dtype) ALLOCATOR_MAKE(_alloc, 1024); ps(dtype_to_str((dtype), &_alloc));
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
