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
#include "str.h"
#include "typedefs.h"
#include "list.h"
#include "error.h"
#include "macho-context.h"
#include "types.h"

void parse(str src) {
    macho_context_init();
    ls_new_char(&strings, 1024, "string literals");
    ls_new_resolv(&resolves, 1024, "deferred address calculations");

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

    int tmp_put_ident = 0;
    bool tmp_put_label = false;
    int tmp_put_idx = 0;
    str tmp_defer_rets = str_empty;
    int tmp_to_resolve = 0;

loop:;
    if (*it.data is '\n') {
        ident = 0;
        while (it.data[1] is ' ') {
            ++it.data;
            ++ident;
        }
    }
    int c = Next();
    bool token_consumed = false;

    TokenStart;
    ReadToken;
    TokenEnd;

    if (tmp_put_label) {
        if (tmp_put_ident > ident) {
            printd("put label here!(%d, %d)\n", tmp_put_ident, ident);
            tmp_put_ident = 0;
            tmp_put_label = false;
            char *ret;
            asprintf(&ret, "__anonyn_%d", tmp_put_idx++);
            tmp_defer_rets = (str){ .data = ret, .len = strlen(ret) };
        }
    }

    if (tmp_defer_rets.data != NULL) {
        int diff = s.code.count - tmp_to_resolve;

        s.code.data[tmp_to_resolve] |= ((diff) << 5); // TODO this only resolves imm19 for cbz, b.cond
        tmp_to_resolve = 0;
        macho_stab_loc(s.code.count * sizeof (u32), tmp_defer_rets);
        free(tmp_defer_rets.data);
        tmp_defer_rets = str_empty;
    }

    if (ident % 4 != 0) {
        CompileErr("Syntax error: single indentation should consist of 4 spaces (was %d)", ident);
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

    if (str_equal_c(token, "struct")) {
        printd("a struct..");

        c = Next();
        TokenStart;
        ReadToken;
        TokenEnd;

        printd("name of '"), strprint_nl(token), printd("'");
        if (Next() == '{') {
            Next(), c = Next();
            if (Is("    ")) {
loop_read_type:
                read_type(&it, &c);
                TokenStart;
                ReadToken;
                TokenEnd;
                printf("name is '"), strprint_nl(token), printf("', ");
                c = Next();
                while (c is '\n' or c is ' ') {
                    c = Next();
                }
                if (c == '}') {
                    printd("end of struct\n");
                    goto brk;
                }
                printf("followed by...");
                goto loop_read_type;
            } else {
                CompileErr("Syntax error: indentation required after struct.");
            }
        } else {
            CompileErr("Syntax error: { expexted after the struct declaration");
        }
    brk:

        printd("\n");
        goto loop;
    }

    if (*token.data != '\n' && token.len != 1) {
        printd("token '"), strprint(token), printd("' (len: %zu, ident: %d, c: %c%d): ", token.len, ident, c, c);
    }

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
        // c = Next();
        printf("static label '"), strprint_nl(token); printf("', ");


        fat f = { _objcode, objcode };
        if (it.data[1] == '(') {
            Next();
            printf("it's a routine, args ");
            macho_stab_ext(f, token);
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
        } else {
            printd("add to local");
            if (token.len is 0) {
                char *ret;
                asprintf(&ret, "__anonyn_%d", tmp_put_idx++);
                token = (str){ .data = ret, .len = strlen(ret) };
            }
            macho_stab_loc(s.code.count * sizeof (u32), token);

            goto loop;
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

    if (!main_defined && token.len != 0 && depth == 0) {
        fat f = { _objcode, objcode };
        printf("main defined here\n");
        macho_stab_ext(f, str_from_c("_main"));
        main_defined = true;
    }

    cflags cond = COND_NV;
    if (str_equal_c(token, "is ")) {
        cond = COND_EQ;
    } else if (str_equal_c(token, "isnt ")) {
        cond = COND_NE;
    }

    if (cond != COND_NV) {
        printd("compare..");

        u8 reg = 0;
        sf_t sf = W; // TODO reg size, reg, pcrel->need to depend on return type..type checking needed

        if (target_nreg) {
            printd("target_nreg was: %d", target_nreg->reg);
            reg = target_nreg->reg;
            sf = nreg_sf(target_nreg);
        } else {
            printd("target_nreg was: null..");
            char *rewind = token.data - 1;
            while (IsSpace(*rewind)) {
                --rewind;
            }
            printd("last thing was.. %d(%c), ", *rewind, *rewind);
            if (*rewind is '>')
                reg = 0;
            else if (*rewind is ']')
                reg = 8;
        }

        if (Is(" 0 ")) {
            printd("special case 0..");
            TokenStart;
            ReadToken;
            TokenEnd;
            strprint_nl(token);
            if (token.len > 0 && Is("->")) {
                printd("..b.cond..");
                tab_find find = stab_search(&stab_loc, token);
                int offset = 0;
                if (find.find isnt NULL) {
                    u64 target = stab_loc.data[find.symbolnum].n_value;
                    offset = target - s.code.count * sizeof (u32);
                } else {
                    resolv tmp = { .name = token, .offset = s.code.count };
                    ls_add_resolv(&resolves, tmp);
                }
                if (offset < 0) {
                    const int IMM19_MINUS2 = 0x1ffffb;
                    offset = IMM19_MINUS2 + offset;
                }
                printd("offset was: %d\n", offset);
                ls_add_u32(&s.code, cbz(sf, reg, offset));
            } else {
                CompileErr("Compile Error in 0 branch\n");
            }
        } else {
            c = Next();

            bool is_minus = false;
            int number = 0;
            bool is_number_set = false;

            TokenStart;
            ReadToken;
            TokenEnd;

            if (c is '-' && IsNum(token.data[1])) {
                printd("negative..");
                is_minus = true;
                c = Next();
                TokenStart;
                ReadToken;
                TokenEnd;
                c = token.data[0];
            } else if (c is '\'') {
                printd("char..");
                c = Next();
                number = (int)c;
                is_number_set = true;
                c = Next();
                c = Next();
            }
            strprint(token);
            if (IsNum(token.data[0])) {
                number = strtol(token.data, &it.data, 10);
                is_number_set = true;
            } else if (!is_number_set) {
                CompileErr("Compile Error: Number expected, was %d", c);
                CompileErr("next was %d", it.data[1]);
            }
            printd("with %d..", number);
            if (target_nreg isnt NULL) {
                printd("target_nreg: %d, %d", target_nreg->reg, target_nreg->size);
            }
            ls_add_u32(&s.code, cmp(sf, reg, number, is_minus));

            c = Next();
            TokenStart;
            ReadToken;
            TokenEnd;
            if (token.len > 0) {
                printd("branch to..");
                strprint(token);
                resolv tmp = { .name = token, .offset = s.code.count };
                ls_add_resolv(&resolves, tmp);
                ls_add_u32(&s.code, b_cond(0, cond));
            } else {
                printd("anonymous label..");
                tmp_to_resolve = s.code.count;
                ls_add_u32(&s.code, b_cond(0, cflags_flip(cond)));    // tmp offset needs to be done later..
                tmp_put_ident = ident;
                tmp_put_label = true;
            }

            if (!Is("->")) {
                CompileErr("Syntax Error: -> expected, but found %d", c);
                CompileErr("next was %d", it.data[1]);
            }
            printd("next: %d", it.data[0]);
            if (it.data[0] is '\n') {
                printd("followed by newline..");
                if (tmp_put_label)
                    tmp_put_ident += 4;
            }
        }
        printd("end\n");
        goto loop;
    }

    char tokc = token.data[0];
    if (tokc is '_' or IsAlpha(tokc)) {

        str name = token;

        if (Is(" :: ")) {
            printf("..is expression\n");

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
            if (Is("i64")) {
                printd("type i64 ");
                c = Next();
                if (!n.is_addr)
                    n.size = 64;
            } else if (Is("i32")) {
                printd("type i32 ");
                c = Next();
                if (!n.is_addr)
                    n.size = 32;
            } else if (Is("c8")) {
                printd("type c8");
                c = Next();
                if (!n.is_addr)
                    n.size = 8;
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
                bool is_stack_alloc = memcmp(line_end - 2, "=[]", 3) == 0;
                printd("line end was %c, is_stack_alloc %d..", line_end[-2], is_stack_alloc);

                if (is_stack_alloc) {
                    n.is_addr = stack_addr;
                }

                if (n.is_addr isnt stack_addr && !is_stack_alloc) {
                    printd("inc reg idx..");
                    s.regs_to_save[s.regs_to_save_size++] = n.reg;
                    named_reg_idx += 1;
                }
                ls_add_nreg(&s.named_regs, n);
                putchar('\n');
            }
            target_nreg = &n;
            iter_prev(&it);
            goto loop;
        }

        if (Is("=>")) {
            printd("branch linked to "), strprint(token);
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
        if (Is("->")) {
            printd("branch to.."), strprint(token);
            tab_find find = stab_search(&stab_loc, token);

            fat f = { _objcode, objcode };
            if (find.find == NULL) {
                macho_relocent(f, &s, find.symbolnum + 1, ARM64_RELOC_BRANCH26, true);
            } else {
                macho_relocent(f, &s, find.symbolnum, ARM64_RELOC_BRANCH26, true);
                // ls_add_int(&to_push_ext, relocents.count - 1);
            }
            printf("find reloc '%p%s <%d>,", find.find, find.find, find.symbolnum);
            printf("'\n");

            ls_add_u32(&s.code, B);

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

    if (*it.data is '[') {
        printd("load...");
        c = Next();
        TokenStart;
        ReadToken;
        TokenEnd;
        printd("from "), strprint_nl(token);

        obj *target = obj_find(&s, token);
        size_t off = target->offset;
        ls_add_u32(&s.code, ldr_size(reg, SP, off, (target->size / 8)));

        c = Next();
        if (it.data[0] is ' ') {
            printd("hi");
            // c = Next();
        }
        printd("..end\n");
        token_consumed = true;
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
            size_t siz_bits = target_nreg->size;
            size_t siz_bytes = siz_bits / 8;
            printd("new allocation.. from %d, %zu..", reg, siz_bits);
            sf_t reg_sf = nreg_sf(target_nreg);
            size_t offset = s.stack_size - s.spaces_left;
            printd("stack_size =0x%zx, spaces_left=0x%zx.., siz=0x%zx", s.stack_size, s.spaces_left, siz_bytes);
            ls_add_u32(&s.code, str_imm(reg_sf, reg, SP, offset));
            if (s.spaces_left < siz_bytes) {
                usize added_size = Align0x10(siz_bytes); // TODO make this packed and efficient
                s.stack_size += added_size;
                s.spaces_left += added_size;
            }
            s.spaces_left -= siz_bytes;
            obj o = {
                .size = siz_bits,
                .is_addr = target_nreg->is_addr,
                .name = target_nreg->name,
                .offset = offset,
            };
            printd("-->after stack_size =0x%zx, spaces_left=0x%zx..", s.stack_size, s.spaces_left);
            ls_add_obj(&s.objects, o);
            putchar('\n');
            goto loop;
        }

        int reg = str_equal_c(before, "=>") ? 0 : 8; // TODO have to accept named registers..

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

    bool is_minus = false;
    if (tokc is '-' && IsNum(token.data[1])) {
        printd("minus..");
        is_minus = true;
        c = Next();
        TokenStart;
        ReadToken;
        TokenEnd;
        tokc = token.data[0];
        strprint(token);
    }
    if (IsNum(tokc)) {
        long number = strtol(token.data, &it.data, 10);
        if (is_minus)
            number = -number;
        sf_t sf = W;
        if (target_nreg)
            sf = nreg_sf(target_nreg);
        ls_add_u32(&s.code, mov(sf, reg, number));
        token_consumed = true;
        printf("value of '%ld to r%d(sf=%d)'", number, reg, sf);
    }

    if (IsAlpha(tokc)) {
        nreg *find = nreg_find(&s, token);

        if (find != NULL) {
            if (find->is_addr == stack_addr) {
                obj *find = obj_find(&s, token);
                ls_add_u32(&s.code, add_imm(X, reg, SP, find->offset));
            } else {
                sf_t sf = nreg_sf(find);
                ls_add_u32(&s.code, mov_reg(sf, reg, find->reg));
            }
            printf("..move to reg %d", reg);
            token_consumed = true;
        } else {
            CompileErr("Error: unknown named register "), PrintErrStr(token);
        }
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
        // printf("\\n\n");
        // ident = 0;
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


    printd("local resolves\n");
    for (int i = 0; i < resolves.count; ++i) {
        str name = resolves.data[i].name;
        tab_find find = stab_search(&stab_loc, name);
        if (find.find is NULL) {
            CompileErr("Compile Error: Could not resolve symbol: "), strprint(name);
        } else {
            printd("found.."), strprint_nl(name);
            stabe entry = stab_loc.data[find.symbolnum];
            u32 offset = resolves.data[i].offset;

            u32 objcode_len = fat_len((fat){_objcode, objcode});
            int diff = (entry.n_value / 4) - offset - objcode_len;
            printd("..diff %d", diff);

            if (diff < 0) {
                const int IMM19_MINUS = 0x80000;
                diff = IMM19_MINUS + diff;
            } else {
                printd("diff wasnt minus");
            }
            printd("..diff was %d..", diff);
            s.code.data[offset] |= ((diff) << 5); // TODO this only resolves imm19 for cbz
            printd("ok\n");
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

    const long offset = (objcode - (void *)_objcode) - (prologue_len * sizeof (u32));
    for (int i = 0; i < to_push.count; ++i) {
        int index = to_push.data[i];
        stab_loc.data[index].n_value += offset;
    }
    for (int i = 0; i < stab_loc.count; ++i) {
        stab_loc.data[i].n_value += prologue_len * sizeof (u32);
    }

    write_buf(&objcode, strings.data, strings.count);

    stack_context_free(&s);
    printf("\nparse end\n");
}

