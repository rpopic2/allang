#include "lexer.h"
#include "slice.h"
#include <stdlib.h>

#define DEBUG(X, ...) printf(X, ...)

int i = 0;
str src;

inline char next() {
    return src.data[++i];
}

int skip_space() {
    int len = 0;
    for (; i < src.len; ++i, ++len) {
        if (IsSpace(src.data[i])) {
            break;
        }
    }
    return len;
}

str read_symbol() {
    str symbol = {0};
    if (IsAlpha(src.data[i])) {
        symbol.data = src.data + i;
        symbol.len = skip_space();

        printf("symbol len %zd data ", symbol.len);
        strprint(symbol);
        printf(" \n");
    }
    return symbol;
}

bool is_two_colons() {
    if (src.data[++i] == ':' && src.data[++i] == ':') {
        if (src.data[++i] != ' ') {
            printf("syntax error: space required after ::\n");
        }
        printf("is a named reg ");
        return true;
    }
    return false;
}

void read_num() {
    if (IsNum(src.data[++i])) {
        str num = { src.data + i };
        num.len = skip_space();

        char *end = num.data + num.len;
        long number = strtol(num.data, &(end), 10);
        printf("with number %ld\n", number);
    }
}

void parse2(str _src) {
    src = _src;
start:;
    str symbol = read_symbol();
    if (symbol.len == 0) {
        // printf("len was 0");
        read_num();
    } else {
        if (is_two_colons()) {
            read_num();
        } else if (src.data[++i] == '\n') {
            printf("mov to scratch register\n");
        }
    }

    if (i < src.len)
        goto start;
}

