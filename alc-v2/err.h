#pragma once

void compile_err(const char *format, ...);
void compile_warning(const char *format, ...);

#define printd(...) printf(__VA_ARGS__)
#define str_printd(...) str_print(__VA_ARGS__)

