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

loop:;
    int c = Next();

    if (c is '_' or IsAlpha(c)) {
        TokenStart
        ReadUntilSpace();
        TokenEnd

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
            }
        } else if (c == '\n') {
            printf("load to stack");
        }
    }

    if (IsNum(c)) {
        TokenStart
        ReadUntilSpace();
        TokenEnd

        long number = strtol(token.data, &it.data, 10);
        printf("value of '%ld'\n", number);

        int reg;
        char nextchar = *(it.data + 1);
        if (nextchar == '\0') {
            reg = 0;
        } else {
            reg = 8;
        }


        fat_put(&stackcode, mov(reg, number));
    }

    if (c == ' ' || c == '\n')
        goto loop;

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

