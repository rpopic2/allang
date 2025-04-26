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

void parse(str src) {
    macho_context_init();
    ls_new_char(&strings, 1024, "string literals");

    parse_scope(src);
}

void parse_scope(str src) {
    printf("start parse\n");
    str_iter it = into_iter(src);
    str token = {};

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

    if (line_end + 1 < it.data) {
        target_nreg = NULL;
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

    printd("token '"), strprint_nl(token), printd("' (len: %zu, ident: %d, c: %c%d): ", token.len, ident, c, c);

    if (Is("//")) {
        printd("comment\n");
        ReadUntilNewline();
        goto loop;
    }

    if (it.data[0] == ':') {
        c = Next();
        if (c != ' ' && c != '\n') {
            CompileErr("Syntax error: space or newline required after a label: ");
        }
        c = Next();
        printf("static label '"), strprint_nl(token); printf("', ");

        fat f = { _objcode, objcode };
        macho_stab_ext(f, token);

        if (c == '(') {
            printf("it's a routine, args ");
            Next();
            TokenStart;
            ReadToken;
            TokenEnd;
            // printf("arg1: "), strprint(token), printf(";");
read_type:
            if (Is("i32")) {    // TODO more types, it does nothing
                printf("type of i32 ");
            } else if (Is("c8")) {
                printf("type of c8 ");
            } else if (Is("FILE")) {
                printf("type of FILE ");
            } else if (Is("addr")) {
                Next();
                printf("type of addr ");
                goto read_type;
            }
            printf(":%x:", it.data[0]);
            if (Is(", ")) {
                goto read_type;
            }
            if (Is(" =>")) {
                printf("return ");
                goto read_type;
            }
        }

        c = Next(); // )
        c = Next(); // \n
        str nextsrc = (str){ it.data };

        for (int i = nextsrc.len; i < src.len; ++i) {
            if (Next() == '\n') {
                c = it.data[1];
                if (c != ' ' && c != '\n')
                    break;
            }
        }

        ++depth;
        printd("\n*** new stack of depth %d\n", depth);
        nextsrc.len = it.data - nextsrc.data;
        strprint(nextsrc);
        printf("end src\n");

        parse_scope(nextsrc);
        --depth;
        printf("\n*** endofrt\n");

        goto loop;
    }

    if (c is ')' or c is '(') {
        goto loop; // TODO maybe do sth useful?
    }
    if (!main_defined && depth == 0) {
        fat f = { _objcode, objcode };
        macho_stab_ext(f, str_from_c("_main"));
        printf("main defined here\n");
        main_defined = true;
    }

    if (str_equal_c(token, "is ")) {
        printd("found compare ");
        if (Is(" 0 ->")) {
            printd("special case 0 ");
            ls_add_u32(&s.code, cbnz(X, 0, 4)); // TODO reg, pcrel
        }
        printd("\n");
        goto loop;
    }

    char tokc = token.data[0];
    if (tokc is '_' or IsAlpha(tokc)) {

        str name = token;

        if (Is(" :: ")) {
            printf("..is named reg\n");

            nreg n = {
                .name = name, .size = 32, .is_addr = not_addr,
            };
            if (Is("stack")) {
                printd("type stack ");
                c = Next();
                n.size = 64;
                n.is_addr = stack_addr;
            } else if (Is("addr")) {
                printd("type addr ");
                c = Next();
                n.size = 64;
                n.is_addr = addr_addr;
            }
            if (Is("i64")) { // TODO impl other types
                printd("type i64 ");
                c = Next();
                if (!n.is_addr)
                    n.size = 64;
            } else if (Is("i32")) {
                printd("type i32 ");
                c = Next();
                if (!n.is_addr)
                    n.size = 32;
            }

            nreg *find = nreg_find(&s, token);
            n.reg = named_reg_idx;
            if (find != NULL) {
                n.reg = find->reg;
                *find = n;
                printf("found "), strprint(find->name);
            }
            if (n.reg > 28) {
                CompileErr("Error: used up all callee-saved registers\n");
            }

            if (find == NULL) {
                s.regs_to_save[s.regs_to_save_size++] = n.reg;
                ls_add_nreg(&s.named_regs, n);
                named_reg_idx += 1;
                putchar('\n');
            }
            target_nreg = &n;
            iter_prev(&it);
            goto loop;
        }

        if (Is("=>")) {
            printf("branch linked to "), strprint(token);
            calls_fn = true;
            tab_find find = stab_search(&stab_ext, token);

            strprint(token);
            fat f = { _objcode, objcode };
            if (find.find == NULL) {
                u32 symbolnum = stab_und.count;
                macho_stab_undef(token);
            printf("failed <%d>,", symbolnum);
                macho_relocent_undef(f, &s, symbolnum, ARM64_RELOC_BRANCH26, true);
            } else {
                macho_relocent(f, &s, find.symbolnum, ARM64_RELOC_BRANCH26, true);
                ls_add_int(&to_push_ext, relocents.count - 1);
            }
            printf("find reloc '%p%s <%d>,", find.find, find.find, find.symbolnum);
            printf("'\n");

            ls_add_u32(&s.code, BL);

            if (is_target_nreg(&it)) {
                printd("target: %d", target_nreg->size);
                sf_t sf = nreg_sf(target_nreg);
                ls_add_u32(&s.code, mov_reg(sf, target_nreg->reg, 0));
            }

            goto loop;
        }
    }


// regs
    int reg;

    if (is_target_nreg(&it)) {
        reg = target_nreg->reg;
    } else {
        if (regoff > 7) {
            CompileErr("Error: used up all scratch registers\n");
        }
        if (line_end + 2 >= (src.data + src.len) || (*line_end == '>')) {
            reg = regoff;
        } else {
            reg = 8 + regoff;
        }
    }

    if (Is("=[")) {

        printd("store ");
        str before = { token.data - 3, 2 };
        TokenStart;
        ReadToken;
        TokenEnd;
        strprint(token);
        printd("toklen: %zu..", token.len);

        if (token.len is 0) {
            size_t siz = target_nreg->size;
            printd("new allocation.. from %d, %zu..", reg, siz);
            sf_t reg_sf = nreg_sf(target_nreg);
            ls_add_u32(&s.code, str_imm(reg_sf, reg, SP, s.stack_size));
            s.stack_size += 0x10; // TODO need to align and scale by 0x10
            // s.stack_size += siz / 8;// the type before..?
            printd("stack_size =0x%zx..", s.stack_size);
            obj o = {
                .size = siz,
                .is_addr = target_nreg->is_addr,
                .name = target_nreg->name,
                .offset = s.obj_offset,
            };
            ls_add_obj(&s.objects, o);
            putchar('\n');
            goto loop;
        }

        strprint(before);
        int reg = str_equal_c(before, "=>") ? 0 : 8; // TODO have to accept named registers.. wait design decision

        nreg *find = nreg_find(&s, token);
        if (find == NULL) {
            CompileErr("Error: unknown named register "), PrintErrStr(token);
        }
        sf_t reg_size = nreg_sf(find);
        if (find->is_addr == stack_addr) {

            obj *find = obj_find(&s, token);
            if (find == NULL) {
                CompileErr("Error: unknown named register "), PrintErrStr(token);
            }

            ls_add_u32(&s.code, str_imm(reg_size, reg, SP, find->offset));
        } else if (find->is_addr == addr_addr) {
            ls_add_u32(&s.code, str_imm(reg_size, reg, find->reg, 0));
        } else {
            CompileErr("Error: trying to store to non-address register\n");
        }
        c = Next();
        goto loop;
    }

    if (tokc is '+') {
        c = Next();
        printf("add");
        u8 reg1 = 8, reg2 = 9;
        if (s.code.count == 0) {
            reg1 = 0, reg2 = 1;
        }
        ls_add_u32(&s.code, add_reg_shft(W, reg, reg1, reg2));
        token_consumed = true;
    }

    if (IsAlpha(tokc)) {
        nreg *find = nreg_find(&s, token);

        if (find != NULL) {
            if (find->is_addr == stack_addr) {
                ls_add_u32(&s.code, add_reg_ext(X, reg, SP, find->reg));
            } else {
                sf_t sf = nreg_sf(find);
                ls_add_u32(&s.code, mov_reg(sf, reg, find->reg));
            }
            printf("..move to reg %d", reg);
            token_consumed = true;
        } else {
            CompileErr("Error: unknown named register "), PrintErrStr(token);
        }
    } else if (IsNum(tokc)) {
        long number = strtol(token.data, &it.data, 10);
        sf_t sf = W;
        if (target_nreg)
            sf = nreg_sf(target_nreg);
        ls_add_u32(&s.code, mov(sf, reg, number));
        token_consumed = true;
        printf("value of '%ld to r%d(sf=%d)'", number, reg, sf);
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
            ls_add_char(&strings, '\0');
        }
        c = Next();
        printf("\n");

        int stab_idx = stab_loc.count - 1;
        fat f = { _objcode, objcode };
        macho_relocent(f, &s, stab_idx, ARM64_RELOC_PAGE21, true);
        ls_add_u32(&s.code, adrp(reg));

        macho_relocent(f, &s, stab_idx, ARM64_RELOC_PAGEOFF12, false);
        ls_add_u32(&s.code, add_imm(X, reg, reg, 0));
        token_consumed = true;
    }

    if (*it.data == ',') {
        // if (target_nreg != NULL) { // TODO
        //     CompileErr("Syntax Error: not allowed to assign two values to one register.");
        // }
        ++regoff;
        Next();
        printf(", ");
        token_consumed = true;
        goto loop;
    }
    regoff = 0;

    if (token.len == 0 && (c == '\n' || c == '\0'))
        token_consumed = true;
    if (!token_consumed && !(c == '\n' && it.data[1] == '\0'))
        CompileErr("token may be not consumed %d", c), strprint(token), printf("\n");
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


    // if (s.epilogue.count == 0) {
    //     ls_add_u32(&s.epilogue, mov(0, 0));
    // }
    if (calls_fn) {
        s.regs_to_save[s.regs_to_save_size++] = 29;
        s.regs_to_save[s.regs_to_save_size++] = 30;
    }
    for (int i = 0; i < s.regs_to_save_size; ++i) {
        u8 reg = s.regs_to_save[i];
        printf("stack size: 0x%zx", s.stack_size);
        if ((i + 1) < s.regs_to_save_size) {
            u8 reg2 = s.regs_to_save[++i];

            u32 op = stp_pre(X, reg, reg2, SP, s.stack_size);
            ls_add_u32(&s.prologue, op);

            op = ldp_post(X, reg, reg2, SP, s.stack_size);
            ls_add_u32(&s.epilogue, op);
            s.stack_size += 0x10;
        } else {
            ls_add_u32(&s.prologue, store(reg, SP, s.stack_size));

            ls_add_u32(&s.epilogue, ldr(reg, SP, s.stack_size));
            s.stack_size += 0x10;
        }

    }

    usize prologue_len = s.prologue.count;
    if (s.stack_size > 0) {
        u32 prologue_sp = sub(SP, SP, s.stack_size);
        write_buf(&objcode, &prologue_sp, sizeof (prologue_sp));
        ++prologue_len;
        ls_add_u32(&s.epilogue, add_imm(X, SP, SP, s.stack_size));
    }

    usize prologue_size = prologue_len * sizeof (u32);
    for (int i = 0; i < s.relocents.count; ++i) {
        u32 index = s.relocents.data[i];
        relocents.data[index].r_address += prologue_size;
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
    printf("\nparse end\n");
}

