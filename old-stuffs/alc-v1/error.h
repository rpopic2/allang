#pragma once

#include "str.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#define printd(...) printf(__VA_ARGS__)

bool has_compile_err = false;

void CompileErr(const char *s, ...) {
    printf("\x1B[31m\n");
    va_list argptr;
    va_start(argptr, s);
    vfprintf(stderr, s, argptr);
    va_end(argptr);
    has_compile_err = true;
    printf("\x1B[0m");
}

void PrintErrStr(str s) {
    printf("\x1B[31m");
    strprint(s);
    printf("\x1B[0m\n");
}
