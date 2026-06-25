#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>

void abort(void);

typedef struct str {
    const char *data;
    const char *end;
} str;

typedef struct iter {
    char *cur;
    char *start;
    char *end;
} iter;

static const str str_null = {.data = 0, .end = 0};

inline static size_t str_len(str s) {
    if (s.end - s.data < 0)
        return 0;
    return (size_t)(s.end - s.data);
}

inline static bool str_empty(const str *s) {
    return s->end == s->data;
}

inline static bool str_eq(const str lhs, const str rhs) {
    return str_len(lhs) == str_len(rhs)
        && memcmp(lhs.data, rhs.data, str_len(lhs)) == 0;
}

inline static bool str_eq_lit(const str s, const char *restrict cstr) {
    return str_len(s) == strlen(cstr) && memcmp(s.data, cstr, strlen(cstr)) == 0;
}

inline static str str_move(str *s) {
    str ret = *s;
    *s = str_null;
    return ret;
}

inline static bool str_to_cstr(str s, char *buf, size_t bufsize) {
    size_t len = str_len(s);
    if (len >= bufsize)
        return false;
    memcpy(buf, s.data, len);
    buf[len] = '\0';
    return true;
}

inline static bool str_ends_with(const str *restrict token, const char *restrict cstr) {
    size_t len = strlen(cstr);
    return str_len(*token) >= len && memcmp(token->end - len, cstr, len) == 0;
}

#define streq(s, t) (memcmp((s), (t), strlen((t))) == 0)

static inline void str_fprintnl(const str s, FILE *file) {
    if (s.data == s.end) {
        fputs("(empty)", file);
    } else {
        if (str_len(s) > 0)
            fwrite(s.data, sizeof (char), (size_t)str_len(s), file);
    }
}

inline static void str_fprint(const str s, FILE *file) {
    str_fprintnl(s, file);
    fputc('\n', file);
}

inline static void str_printnl(const str s) {
    str_fprintnl(s, stdout);
}

inline static void str_print(const str s) {
    str_fprintnl(s, stdout);
    fputc('\n', stdout);
}

inline static str str_from_iter(const iter *it) {
    return (str){.data = it->start, it->cur};
}

inline static iter iter_init(char *start, size_t end) {
    return (iter){.start = start, .cur = start, .end = start + end};
}

enum str_fmt_len {
    STR_FMT_NONE, STR_FMT_HH, STR_FMT_H, STR_FMT_L, STR_FMT_LL,
    STR_FMT_J, STR_FMT_Z, STR_FMT_T, STR_FMT_BIGL,
};

static void str_vfprintf(FILE *const file, const str fmt, va_list args) {
    const char *p = fmt.data;
    const char *const end = fmt.end;
    while (p < end) {
        if (*p != '%') {
            const char *const run = p;
            while (p < end && *p != '%')
                ++p;
            fwrite(run, sizeof (char), (size_t)(p - run), file);
            continue;
        }

        char spec[64];
        size_t si = 0;
        spec[si++] = *p++;

        while (p < end && si < sizeof spec - 1 && strchr("-+ #0", *p))
            spec[si++] = *p++;
        if (p < end && *p == '*') {
            const int width = va_arg(args, int);
            si += (size_t)snprintf(spec + si, sizeof spec - si, "%d", width);
            ++p;
        } else {
            while (p < end && si < sizeof spec - 1 && isdigit((unsigned char)*p))
                spec[si++] = *p++;
        }
        if (p < end && *p == '.') {
            spec[si++] = *p++;
            if (p < end && *p == '*') {
                const int prec = va_arg(args, int);
                si += (size_t)snprintf(spec + si, sizeof spec - si, "%d", prec);
                ++p;
            } else {
                while (p < end && si < sizeof spec - 1 && isdigit((unsigned char)*p))
                    spec[si++] = *p++;
            }
        }

        enum str_fmt_len length = STR_FMT_NONE;
        if (p + 1 < end && p[0] == 'h' && p[1] == 'h') {
            length = STR_FMT_HH, spec[si++] = *p++, spec[si++] = *p++;
        } else if (p < end && *p == 'h') {
            length = STR_FMT_H, spec[si++] = *p++;
        } else if (p + 1 < end && p[0] == 'l' && p[1] == 'l') {
            length = STR_FMT_LL, spec[si++] = *p++, spec[si++] = *p++;
        } else if (p < end && *p == 'l') {
            length = STR_FMT_L, spec[si++] = *p++;
        } else if (p < end && *p == 'j') {
            length = STR_FMT_J, spec[si++] = *p++;
        } else if (p < end && *p == 'z') {
            length = STR_FMT_Z, spec[si++] = *p++;
        } else if (p < end && *p == 't') {
            length = STR_FMT_T, spec[si++] = *p++;
        } else if (p < end && *p == 'L') {
            length = STR_FMT_BIGL, spec[si++] = *p++;
        }

        if (p >= end) {
            fwrite(spec, sizeof (char), si, file);
            break;
        }

        const char conv = *p++;
        spec[si++] = conv;
        spec[si] = '\0';
        switch (conv) {
        case '%':
            fputc('%', file);
            break;
        case 'S':
            str_fprintnl(va_arg(args, str), file);
            break;
        case 'd':
        case 'i':
            switch (length) {
            case STR_FMT_L:
                fprintf(file, spec, va_arg(args, long));
                break;
            case STR_FMT_LL:
            case STR_FMT_J:
                fprintf(file, spec, va_arg(args, long long));
                break;
            case STR_FMT_Z:
                fprintf(file, spec, va_arg(args, size_t));
                break;
            case STR_FMT_T:
                fprintf(file, spec, va_arg(args, ptrdiff_t));
                break;
            default:
                fprintf(file, spec, va_arg(args, int));
                break;
            }
            break;
        case 'o':
        case 'u':
        case 'x':
        case 'X':
            switch (length) {
            case STR_FMT_L:
                fprintf(file, spec, va_arg(args, unsigned long));
                break;
            case STR_FMT_LL:
            case STR_FMT_J:
                fprintf(file, spec, va_arg(args, unsigned long long));
                break;
            case STR_FMT_Z:
            case STR_FMT_T:
                fprintf(file, spec, va_arg(args, size_t));
                break;
            default:
                fprintf(file, spec, va_arg(args, unsigned int));
                break;
            }
            break;
        case 'c':
            fprintf(file, spec, va_arg(args, int));
            break;
        case 's':
            fprintf(file, spec, va_arg(args, char *));
            break;
        case 'p':
            fprintf(file, spec, va_arg(args, void *));
            break;
        case 'e':
        case 'E':
        case 'f':
        case 'F':
        case 'g':
        case 'G':
        case 'a':
        case 'A':
            if (length == STR_FMT_BIGL)
                fprintf(file, spec, va_arg(args, long double));
            else
                fprintf(file, spec, va_arg(args, double));
            break;
        default:
            fwrite(spec, sizeof (char), si, file);
            break;
        }
    }
}

static void str_fprintf(FILE *const file, const str fmt, ...) {
    va_list args;
    va_start(args, fmt);
    str_vfprintf(file, fmt, args);
    va_end(args);
}

static void str_printf(const str fmt, ...) {
    va_list args;
    va_start(args, fmt);
    str_vfprintf(stdout, fmt, args);
    va_end(args);
}

#define STR(s) (str) { .data = (s), .end = (s) + strlen(s) }
