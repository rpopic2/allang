#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aarch64.h"
#include "file.h"
#include "lexer.h"
#include "slice.h"
#include "typedefs.h"

enum token_type {
    token_none, token_reg,
};


#define Next() iter_next(&it)
#define if_Is(X) if (memcmp((X), it.data, strlen(X)) == 0) { it.data += strlen(X); c = *it.data;
#define is ==
#define or ||

#define ReadUntilSpace() while (c != ' ' && c != '\n' && c != ITER_EOF) { c = Next(); }
#define ReadToken() while (c != ' ' && c != '\n' && c != ',' && c != ITER_EOF) { c = Next(); }


#define TokenStart token.data = it.data;
#define TokenEnd  token.len = it.data - token.data;
#define fat_new(T, X, A) T (_##X)A; fat X = (fat) { _##X, _##X };

u32 _objcode[1024];
writer_t objcode = _objcode;

fat_new(u32, stackcode, [1024]);
fat_new(u32, prologue, [1024]);
fat_new(u32, epilogue, [1024]);

void parse(str src) {
    str_iter it = into_iter(src);
    str token = (str) { NULL, 0 };
    u8 reg_to_save = 0;
    size_t stack_size = 0;
    int regoff = 0;


loop:;
    int c = Next();

    TokenStart
    ReadUntilSpace();
    TokenEnd
    char tokc = token.data[0];

    if (tokc is '_' or IsAlpha(tokc)) {

        printf("token "); printstr(token);

        Next();
        if_Is(":: ")
            printf("..is named reg");
            if (IsNum(c)) {
                TokenStart
                ReadUntilSpace();
                TokenEnd

                long number = strtol(token.data, &it.data, 10);
                printf("..value of '%ld'\n", number);

                reg_to_save = 20;
                fat_put(&stackcode, mov(20, number));
                goto loop;
            }
        }
    }

    char *p = it.data;
    while (*p != '\n' && *p != '\0') {
        ++p;
    }
    char line_end = *(p + 1);

    bool isAlph = IsAlpha(tokc);
num:
    if (IsNum(tokc) || isAlph) {
        long number = strtol(token.data, &it.data, 10);

        int reg;

        if (line_end == '\0') {
            reg = regoff;
        } else {
            reg = 8 + regoff;
        }

        if (isAlph) {
            fat_put(&stackcode, mov_reg(R, reg, 20));
            printf("..move to %d", reg);
        } else {
            fat_put(&stackcode, mov(reg, number));
            printf("value of '%ld'", number);
        }

        if (*it.data == ',') {
            ++regoff;
            printf(", %d", regoff);
            goto num;
        }
        regoff = 0;
        printf("\n");
    }


    if (c == ' ' || c == '\n')
        goto loop;

    if (c != '\0') {
        printf("file not end yet...");
    }

    if (reg_to_save != 0) {
        fat_put(&prologue, sub(SP, SP, 0x10));
        fat_put(&prologue, store(reg_to_save, SP, 0x8));

        fat_put(&epilogue, ldr(reg_to_save, SP, 0x8));
        fat_put(&epilogue, add(SP, SP, 0x10));
        reg_to_save = 0;
    }

    fat_put(&epilogue, RET);

    // memcpy(objcode, prologue, prologue - _prologue);

    write_buf_fat(&objcode, prologue);
    write_buf_fat(&objcode, stackcode);
    write_buf_fat(&objcode, epilogue);
    // size_t scsize = fat_len(stackcode);
    // memcpy(objcode, _stackcode, scsize * 4);
    // objcode += scsize;

    // memcpy(objcode, epilogue, epilogue - _epilogue);
    printf("parse end\n");
}

