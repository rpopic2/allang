#pragma once
#include <stdbool.h>

static inline bool IsNum(char X) {
    return X >= '0' && X <= '9';
}

static inline bool IsUpper(char X) {
    return X >= 'A' && X <= 'Z';
}

static inline bool IsLower(char X) {
    return X >= 'a' && X <= 'z';
}

static inline bool IsAlpha(char X) {
    return IsLower(X) || IsUpper(X);
}

static inline bool IsSpace(char c) {
    return c == ' ' || c == '\n';
}
