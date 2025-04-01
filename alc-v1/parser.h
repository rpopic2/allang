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
#include "parser_util.h"
#include "slice.h"
#include "typedefs.h"
#include "list.h"
#include "error.h"
#include "macho-context.h"
#include "stack_context.h"

void parse(str src) {
    macho_context_init();
    ls_new_char(&strings, 1024, "string literals");

    parse_scope(src);
}

void parse_scope(str src) {
    printf("start parse "), strprint(src);
    str_iter it = into_iter(src);
    str token = {};

    u8 reg_to_save = 0;
    u8 named_reg_idx = 19;
    int regoff = 0;
    int ident = 0;

    stack_context s;
    stack_context_new(&s);
    char *line_end = NULL;

    bool calls_fn = false;

loop:;
    int c = Next();
    bool token_consumed = false;

    TokenStart;
    ReadToken;
    TokenEnd;

    if (token.len == 0 && c == ' ') {
        ++ident;
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
    bool is_eof = line_end[2] == '\0';
    if (is_eof) {
        token_consumed = true;
    }

    printdbg("token '"), strprint_nl(token), printdbg("' (len: %zu, ident: %d, c: %c%d): ", token.len, ident, c, c);

    if (it.data[0] == ':') {
        c = Next();
        printf("wow%d", c);
        if (c != ' ' && c != '\n') {
            CompileErr("Syntax error: space or newline required after a label: ");
        }
        c = Next();
        printf("static label '"), strprint_nl(token); printf("', ");

        fat f = { _objcode, objcode };
        macho_stab_ext(f, token);

        if (c == '(') {
            printf("it's a routine, ");

            Next();
            TokenStart;
            ReadToken;
            TokenEnd;
            if (Is("i32")) {
                printf("arg type of i32");
            }
        }

        c = Next(); // )
        c = Next(); // \n
        str nextsrc = (str){ it.data };

        for (int i = nextsrc.len; i < src.len; ++i) {
            c = Next();
            if (c != '\n')
                continue;
            int ident = 0;
            while (c == ' ') {
                Next();
                ++ident;
            }
            printf("has ident %d\n", ident);
            if (ident == 0) {   // TODO need to hanle blank lines
                printf("\n\n\tnew stack\n");
                it.data[0] = '\0';
                nextsrc.len = it.data - nextsrc.data;

                parse_scope(nextsrc);

                printf("endofrt\n\n\n");
                break;
            }
            c = iter_prev(&it);
        }

        goto loop;
    }

    if (!main_defined && ident == 0) {
        fat f = { _objcode, objcode };
        macho_stab_ext(f, str_from_c("_main"));
        printf("main defined here\n");
        main_defined = true;
    }

    char tokc = token.data[0];
    if (tokc is '_' or IsAlpha(tokc)) {

        str name = token;

        if (Is(" :: ")) {
            printf("..is named reg");

            nreg *find = NULL;
            for (int i = 0; i < s.named_regs.count; ++i) {
                nreg *tmp = s.named_regs.data + i;
                if (str_equal(tmp->name, name)) {
                    find = tmp;
                }
            }

            int reg = named_reg_idx;
            if (find != NULL) {
                reg = find->reg;
            }
            if (reg > 28) {
                CompileErr("Error: used up all callee-saved registers\n");
            }
            if (find == NULL) {
                if (reg_to_save != 0) {
                    make_prelude(&s, reg_to_save, reg);
                    reg_to_save = 0;
                } else {
                    reg_to_save = reg;
                }
                ls_add_nreg(&s.named_regs, (nreg) { name, reg });
                named_reg_idx += 1;
            }
            if (find != NULL) {
                printf("found "), strprint(find->name);
            }

            if (IsNum(c)) {
                TokenStart;
                ReadUntilSpace();
                TokenEnd;

                long number = strtol(token.data, &it.data, 10);
                ls_add_u32(&s.code, mov(reg, number));

                printf("..value of '%ld'\n", number);

                goto loop;
            }
        }

        if (Is("=>")) {
            printf("branch linked to "), strprint(token);
            calls_fn = true;
            macho_stab_undef(token);

            fat f = { _objcode, objcode };
            macho_relocent_undef(f, &s.code, ARM64_RELOC_BRANCH26, true);

            ls_add_u32(&s.code, BL);
            goto loop;
        }
    }


    int reg;

    if (regoff > 7) {
        CompileErr("Error: used up all scratch registers\n");
    }
    if (line_end + 2 >= (src.data + src.len) || (*line_end == '>')) {
        reg = regoff;
    } else {
        reg = 8 + regoff;
    }

    if (tokc is '+') {
        c = Next();
        printf("add");
        u8 reg1 = 8, reg2 = 9;
        if (s.code.count == 0) {
            reg1 = 0, reg2 = 1;
        }
        ls_add_u32(&s.code, add_shft(R, reg, reg1, reg2));
        token_consumed = true;
    }

    if (IsAlpha(tokc)) {
        nreg *find = NULL;
        for (int i = 0; i < s.named_regs.count; ++i) {
            nreg *tmp = s.named_regs.data + i;
            if (memcmp(tmp->name.data, token.data, token.len) == 0) {
                find = tmp;
            }
        }

        if (find != NULL) {
            ls_add_u32(&s.code, mov_reg(R, reg, find->reg));
            printf("..move to %d", reg);
        } else {
            CompileErr("Error: unknown named register "), PrintErrStr(token);
        }
    } else if (IsNum(tokc)) {
        long number = strtol(token.data, &it.data, 10);
        ls_add_u32(&s.code, mov(reg, number));
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

        fat f = { _objcode, objcode };
        macho_relocent(f, &s.code, ARM64_RELOC_PAGE21, true);
        ls_add_u32(&s.code, adrp(reg));

        macho_relocent(f, &s.code, ARM64_RELOC_PAGEOFF12, false);
        ls_add_u32(&s.code, add(X, reg, reg, 0));
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


    if (calls_fn) {
        s.stack_size += 0x10;
        u32 op = stp_pre(X, 29, 30, SP, s.stack_size);
        ls_add_u32(&s.prologue, op);

        op = ldp_post(X, 29, 30, SP, s.stack_size);
        ls_add_u32(&s.epilogue, op);
    }
    if (reg_to_save != 0) {
        s.stack_size += 0x10;
        ls_add_u32(&s.prologue, store(reg_to_save, SP, 0x8));

        ls_add_u32(&s.epilogue, ldr(reg_to_save, SP, 0x8));
        reg_to_save = 0;
    }

    usize prologue_len = s.prologue.count;
    if (s.stack_size > 0) {
        u32 prologue_sp = sub(SP, SP, s.stack_size);
        write_buf(&objcode, &prologue_sp, sizeof (prologue_sp));
        ++prologue_len;
        ls_add_u32(&s.epilogue, add(X, SP, SP, s.stack_size));
    }

    usize prologue_size = prologue_len * sizeof (u32);
    for (int i = 0; i < relocents.count; ++i) {
        relocents.data[i].r_address += prologue_size;
    }

    ls_add_u32(&s.epilogue, RET);

    write_buf(&objcode, s.prologue.data, s.prologue.count * sizeof(u32));
    write_buf(&objcode, s.code.data, s.code.count * sizeof(u32));
    write_buf(&objcode, s.epilogue.data, s.epilogue.count * sizeof(u32));

    const long offset = objcode - (void *)_objcode;
    for (int i = 0; i < to_push.count; ++i) {
        int index = to_push.data[i];
        stab_loc.data[index].n_value += offset;
    }

    write_buf(&objcode, strings.data, strings.count);

    stack_context_free(&s);
    printf("parse end\n");
}

