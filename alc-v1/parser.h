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
#include "list.h"


#define Next() iter_next(&it)
#define if_Is(X) if (memcmp((X), it.data, strlen(X)) == 0) { it.data += strlen(X); c = *it.data;
#define is ==
#define or ||

#define ReadUntilSpace() while (c != ' ' && c != '\n' && c != '\0') { c = Next(); }
#define ReadToken() while (c != ' ' && c != '\n' && c != ',' && c != '\0') { c = Next(); }


#define TokenStart token.data = it.data;
#define TokenEnd  token.len = it.data - token.data;
#define fat_new(T, X, A) T (_##X)A; fat X = (fat) { _##X, _##X };

u32 _objcode[1024];
writer_t objcode = _objcode;

fat_new(u32, stackcode, [1024]);
fat_new(u32, prologue, [1024]);
fat_new(u32, epilogue, [1024]);

typedef struct {
    str name;
    u8 reg;
} nreg; // named registers
ls (nreg);

void parse(str src) {
    str_iter it = into_iter(src);
    str token = (str) { NULL, 0 };
    u8 reg_to_save = 0;
    size_t stack_size = 0;
    u8 named_reg_idx = 20;
    int regoff = 0;
    char line_end = '\0';

    lsnreg named_regs;
    ls_new_nreg(&named_regs, 32, "named registers");

loop:;
    int c = Next();

    TokenStart
    ReadToken();
    TokenEnd

    if (token.len == 0) {
        goto fin;
    }

    printf("token "), printstr(token), printf(": ");
    char tokc = token.data[0];

    if (tokc is '_' or IsAlpha(tokc)) {

        str name = token;

        Next();
        if_Is(":: ")
            printf("..is named reg");
            if (IsNum(c)) {
                TokenStart
                ReadUntilSpace();
                TokenEnd

                long number = strtol(token.data, &it.data, 10);
                printf("..value of '%ld'\n", number);

                if (reg_to_save != 0) {
                    u32 op = stp_pre(X, reg_to_save, named_reg_idx, SP, 0);
                    fat_put(&prologue, op);

                    op = ldp_post(X, reg_to_save, named_reg_idx, SP, 0);
                    fat_put(&epilogue, op);
                    stack_size += 0x10;
                    reg_to_save = 0;
                } else {
                    reg_to_save = named_reg_idx;
                }
                ls_add_nreg(&named_regs, (nreg) { name, named_reg_idx });
                fat_put(&stackcode, mov(named_reg_idx, number));
                named_reg_idx += 1;
                goto loop;
            }
        } else {
            iter_prev(&it);
        }
    }


    bool isAlph = IsAlpha(tokc);

    if (IsNum(tokc) || isAlph) {

        int reg;

        if (line_end == '\0') {
            reg = regoff;
        } else {
            reg = 8 + regoff;
        }

        if (isAlph) {
            nreg *find = NULL;
            for (int i = 0; i < named_regs.count; ++i) {
                nreg *tmp = named_regs.data + i;
                if (memcmp(tmp->name.data, token.data, token.len) == 0) {
                    find = tmp;
                }
            }

            if (find != NULL) {
                fat_put(&stackcode, mov_reg(R, reg, find->reg));
                printf("..move to %d", reg);
            } else {
                printf("Error: unknown named register "), printstr(token);
            }
        } else {
            long number = strtol(token.data, &it.data, 10);
            fat_put(&stackcode, mov(reg, number));
            printf("value of '%ld'", number);
        }

        printf("%d", *it.data);
        if (*it.data == ',') {
            ++regoff;
            Next();
            printf(", ");
            goto loop;
        }
        regoff = 0;
        printf("\n");
    }


    if (c == '\n') {
        char *p = it.data;
        while (*p != '\n' && *p != '\0') {
            ++p;
        }
        line_end = *(p + 1);
    }
    if (c == ' ' || c == '\n')
        goto loop;

fin:
    if (c != '\0') {
        printf("might returned prematurely, %s\n", it.data);
    }
    ls_delete_nreg(&named_regs);


    if (reg_to_save != 0) {
        stack_size += 0x10;
        fat_put(&prologue, store(reg_to_save, SP, 0x8));

        fat_put(&epilogue, ldr(reg_to_save, SP, 0x8));
        reg_to_save = 0;
    }
    u32 prologue_sp = sub(SP, SP, stack_size);
    write_buf(&objcode, &prologue_sp, sizeof (prologue_sp));
    fat_put(&epilogue, add(SP, SP, stack_size));

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

