#pragma once

#include <mach-o/arm64/reloc.h>
#include <mach-o/reloc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach-o/nlist.h>

#include "aarch64.h"
#include "file.h"
#include "lexer.h"
#include "slice.h"
#include "typedefs.h"
#include "list.h"
#include "error.h"
#include "macho-context.h"

#define is ==
#define or ||

#define Next() iter_next(&it)
#define if_Is(X) if (memcmp((X), it.data, strlen(X)) == 0) { it.data += strlen(X); c = *it.data;

#define ReadUntilSpace() while (c != ' ' && c != '\n' && c != '\0') { c = Next(); }
#define ReadToken_old() while (c != ' ' && c != '\n' && c != ',' && c != '"' && c != '\0' && c != '=') { c = Next(); }

#define ReadToken() while (IsAlpha(c) || IsNum(c) || c is '.' || c is '_') { c = Next();}

#define printdbg(...) printf(__VA_ARGS__)

#define TokenStart token.data = it.data;
#define TokenEnd  token.len = it.data - token.data;

u32 _objcode[1024];
writer_t objcode = _objcode;

ls_char strings;
bool main_defined;

fat_new(u32, stackcode, [1024]);
fat_new(u32, prologue, [1024]);
fat_new(u32, epilogue, [1024]);

typedef struct {
    str name;
    u8 reg;
} nreg; // named registers
ls (nreg);

void parse_named_reg() {

}

void scope(str src);

void parse(str src) {
    scope(src);
}

void scope(str src) {
    str_iter it = into_iter(src);
    str token = (str) { NULL, 0 };

    u8 reg_to_save = 0;
    u8 named_reg_idx = 19;
    int regoff = 0;
    ls_nreg named_regs;
    int ident = 0;

    macho_context_init();

    size_t stack_size = 0;
    char *line_end = NULL;

    bool calls_fn = false;

    ls_new_nreg(&named_regs, 16, "named registers");
    ls_new_char(&strings, 1024, "string literals");


loop:;
    int c = Next();
    bool token_consumed = false;

    TokenStart;
    ReadToken();
    TokenEnd;

    if (token.len == 0 && (c == ' ')) {
        ++ident;
        goto loop;
    }
    if (token.len == 0 && (c == '\n')) {
        printf("endl\n");
        goto loop;
    }
    if (ident % 4 != 0) {
        CompileErr("Syntax error: single indentation should consist of 4 spaces");
    }

    if (line_end < it.data) {
        char *p = it.data;
        while (*p != '\n' && *p != '\0') {
            ++p;
        }
        line_end = p - 1;
    }
    if (line_end[2] == '\0') {
        token_consumed = true;
    }

    char tokc = token.data[0];
    printdbg("token '"), strprint_nl(token), printdbg("' (len: %zu, ident: %d, c: %c%d): ", token.len, ident, c, c);

    if (it.data[0] == ':') {
        c = Next();
        if (c != ' ' && c != '\n') {
            CompileErr("Syntax error: space or newline required after a label: ");
        }
        c = Next();
        printf("static label '"), strprint_nl(token); printf("', ");
        macho_stab_ext(stackcode, token);

        if (c == '(') {
            printf("it's a routine, ");

            c = Next();
            TokenStart
            ReadToken();
            TokenEnd
            if (streq_c(token, "i32")) {
                printf("arg type of i32");
            }
        }

        goto loop;
    }

    if (!main_defined && ident == 0) {
        macho_stab_ext(stackcode, str_from_c("_main"));
        printf("main defined here\n");
        main_defined = true;
    }

    if (tokc is '_' or IsAlpha(tokc)) {

        str name = token;

        if_Is(" :: ") // {
            printf("..is named reg");
            if (IsNum(c)) {
                TokenStart;
                ReadUntilSpace();
                TokenEnd;

                nreg *find = NULL;
                for (int i = 0; i < named_regs.count; ++i) {
                    nreg *tmp = named_regs.data + i;
                    if (memcmp(tmp->name.data, name.data, name.len) == 0) {
                        find = tmp;
                    }
                }
                if (find != NULL) {
                    printf("found "), strprint(find->name);
                }

                long number = strtol(token.data, &it.data, 10);
                printf("..value of '%ld'\n", number);

                int reg = named_reg_idx;
                if (find != NULL) {
                    reg = find->reg;
                }
                if (reg > 28) {
                    CompileErr("Error: used up all callee-saved registers\n");
                }
                if (find == NULL) {
                    if (reg_to_save != 0) {
                        u32 op = stp_pre(X, reg_to_save, reg, SP, stack_size);
                        fat_put(&prologue, op);
                        op = ldp_post(X, reg_to_save, reg, SP, stack_size);
                        fat_put(&epilogue, op);
                        stack_size += 0x10;
                        reg_to_save = 0;
                    } else {
                        reg_to_save = reg;
                    }
                    ls_add_nreg(&named_regs, (nreg) { name, reg });
                }
                fat_put(&stackcode, mov(reg, number));
                if (find == NULL)
                    named_reg_idx += 1;
                goto loop;
            }
        }

        if_Is("=>") // {
            printf("branch linked to "), strprint(token);
            calls_fn = true;
            macho_stab_undef(token);

            macho_relocent_undef(stackcode, ARM64_RELOC_BRANCH26, true);

            fat_put(&stackcode, BL);
            goto loop;
        }
    }


    int reg;

    if (regoff > 7) {
        CompileErr("Error: used up all scratch registers\n");
    }
    if (line_end[2] == '\0' || (*line_end == '>')) {
        reg = regoff;
    } else {
        reg = 8 + regoff;
    }

    if (tokc is '+') {
        // let's do adding!
        //  0b090100
        c = Next();
        printf("add");
        fat_put(&stackcode, add_shft(R, reg, 8, 9));
        token_consumed = true;
    }

    if (IsAlpha(tokc)) {
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
            CompileErr("Error: unknown named register "), PrintErrStr(token);
        }
    } else if (IsNum(tokc)) {
        long number = strtol(token.data, &it.data, 10);
        fat_put(&stackcode, mov(reg, number));
        token_consumed = true;
        printf("value of '%ld'", number);
    } else if (tokc is '"') {
        // is going to be a string
        c = Next();
        TokenStart
        while (c != '"' && c != '\0') { c = Next(); }
        TokenEnd

        printf("original: `"), strprint_nl(token), printf("` ");
        for (int i = 0; i < token.len; ++i) {
            char d = token.data[i];
            if (d != '\\')
                continue;
            d = token.data[i + 1];
            printf("(has escape seq %c) ", d);
            if (d == 'n') {
                token.data[i] = '\n';
                memmove(token.data + i + 1, token.data + i + 2, token.len - i);
                token.len -= 1;
            }
        }
        printf("str lit: `"), strprint_nl(token), printf("`");

        macho_stab_stringlit();

        ls_addran_char(&strings, token.data, token.len);
        if (it.data[1] == '0') {
            printf("is null terminated string");
            c = Next();
            c = Next();
            ls_add_char(&strings, '\0');
        }
        printf("\n");

        macho_relocent(stackcode, ARM64_RELOC_PAGE21, true);
        fat_put(&stackcode, adrp(reg));

        macho_relocent(stackcode, ARM64_RELOC_PAGEOFF12, false);
        fat_put(&stackcode, add(X, reg, reg, 0));
        token_consumed = true;
    }

    if (*it.data == ',') {
        ++regoff;
        Next();
        printf(", ");
        token_consumed = true;
        goto loop;
    }
    regoff = 0;

    if (!token_consumed && !(c == '\n' && it.data[1] == '\0'))
        printf("token may be not consumed %d", c), strprint(token), printf("\n");
    if (c == '\n') {
        printf("\\n\n");
        ident = 0;
        goto loop;
    }
    if (c != '\0') {
        goto loop;
    }

//fin
    if (c != '\0') {
        CompileErr("parser returned prematurely, %d, '%s'\n", c, it.data);
    }
    ls_delete_nreg(&named_regs);


    if (calls_fn) {
        stack_size += 0x10;
        u32 op = stp_pre(X, 29, 30, SP, stack_size);
        fat_put(&prologue, op);

        op = ldp_post(X, 29, 30, SP, stack_size);
        fat_put(&epilogue, op);
    }
    if (reg_to_save != 0) {
        stack_size += 0x10;
        fat_put(&prologue, store(reg_to_save, SP, 0x8));

        fat_put(&epilogue, ldr(reg_to_save, SP, 0x8));
        reg_to_save = 0;
    }

    usize prologue_len = prologue.end - prologue.start;
    if (stack_size > 0) {
        u32 prologue_sp = sub(SP, SP, stack_size);
        write_buf(&objcode, &prologue_sp, sizeof (prologue_sp));
        ++prologue_len;
        fat_put(&epilogue, add(X, SP, SP, stack_size));
    }

    usize prologue_size = prologue_len * sizeof (u32);
    for (int i = 0; i < relocents.count; ++i) {
        relocents.data[i].r_address += prologue_size;
    }

    fat_put(&epilogue, RET);

    write_buf_fat(&objcode, prologue);
    write_buf_fat(&objcode, stackcode);
    write_buf_fat(&objcode, epilogue);

    const long offset = objcode - (void *)_objcode;
    for (int i = 0; i < to_push.count; ++i) {
        int index = to_push.data[i];
        stab_loc.data[index].n_value += offset;
    }

    write_buf(&objcode, strings.data, strings.count);
    printf("parse end\n");
}

