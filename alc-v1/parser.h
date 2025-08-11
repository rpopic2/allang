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
#include "stack_context.h"
#include "str.h"
#include "typedefs.h"
#include "list.h"
#include "error.h"
#include "macho-context.h"
#include "types.h"

void parse(str src) {
    macho_context_init();
    types_init();
    ls_new_char(&strings, 1024, "string literals");
    ls_new_resolv(&resolves, 1024, "deferred address calculations");

    parse_scope(src, false, str_from_c("_main"));

    write_buf(&objcode, strings.data, strings.count);

    types_destroy();
}

void add_nreg(stack_context *s, str type_name, str param_name, int from_reg, ptype addr_type, int bsize, u8 *named_reg_idx) {
    type_info *t = type_find(type_name);
    nreg n = {
        .name = param_name,
        .type = t,
        .reg = *named_reg_idx,
        .is_addr = addr_type,
        .bsize = bsize
    };
    s->regs_to_save[s->regs_to_save_size++] = n.reg;
    ls_add_nreg(&s->named_regs, n);
    sf_t sf = nreg_sf(&n);
    ls_add_u32(&s->code, mov_reg(sf, n.reg, from_reg));
    *named_reg_idx += 1;
}

int anonyn_index = 0;

void parse_scope(str src, bool isnt_main, str name) {
    printd("start parse\n\n");

    str_iter it = into_iter(src);
    str token = {};

    u8 named_reg_idx = 19;
    int regoff = 0;
    int ident = 0;

    stack_context s;
    stack_context_new(&s);
    char *line_end = NULL;

    bool is_routine = false;
    if (!isnt_main)
        is_routine = true;

    bool calls_fn = false;

    int pending_lable_ident = 4 * depth;
    bool pending_put_label = false;
    str tmp_defer_rets = str_empty;
    int tmp_to_resolve = 0;
    bool tmp_put_label_nextline = false;

    nreg *target_scratch = NULL;

    if (isnt_main) {
        int c = Next();
        if (it.data[1] == '(') {
            printd("\n=> Routine "), strprint_nl(name), printd(": ");
            is_routine = true;
            // fat f = {_objcode, objcode};
            // macho_stab_ext(f, name);

            Next();
            Next();
            TokenStart;
            ReadToken;
            TokenEnd;

            int param_index = 0;
read_type:;
            if (it.data[0] is ')') {
                printd("end\n");
                goto ok;
            }
            type_info *t = read_type(&it, &c);
            if (t is NULL) {
                goto loop;
            }
            strprint_nl(t->name), printd(" ");

            c = Next();
            TokenStart;
            ReadToken;
            TokenEnd;
            strprint_nl(token), printd(" ");

            nreg n = {
                .name = token, .type = t, .reg = named_reg_idx, .bsize = t->bsize
            };
            ls_add_nreg(&s.named_regs, n);
            s.regs_to_save[s.regs_to_save_size++] = n.reg;
            named_reg_idx += 1;

            sf_t sf = nreg_sf(&n);
            ls_add_u32(&s.code, mov_reg(sf, n.reg, param_index));

            param_index += 1;

            if (Is(", ")) {
                goto read_type;
            }
            if (Is(" =>")) {
                printd("returns ");
                goto read_type;
            }
            goto read_type;
        } else {
            printd("=> Local label ");
            if (!main_defined && depth == 1) {
                // fat f = { _objcode, objcode };
                // printd("main defined here\n");
                // macho_stab_ext(f, str_from_c("_main"));
                // main_defined = true;
            }

            if (token.len is 0) {
                char *ret;
                asprintf(&ret, "__anonyn_%d", anonyn_index++);
                token = (str){ .data = ret, .len = strlen(ret) };
            }
            // macho_stab_loc(s.code.count * sizeof (u32), token);

            goto loop;
        }
ok:;
        c = Next(); // should be \n
    }

loop:;
    if (*it.data is '\n' && it.data[1] isnt '\n') {
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

    if (pending_put_label) {
        if (pending_lable_ident > ident) {
            printd("put label here!(%d, %d)\n", pending_lable_ident, ident);
            pending_lable_ident = 0;
            pending_put_label = false;
            char *ret;
            asprintf(&ret, "__anonyn_%d", anonyn_index++);
            tmp_defer_rets = (str){ .data = ret, .len = strlen(ret) };
        }
    }

    if (tmp_defer_rets.data != NULL) {
        printd("actually put here..");
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
        line_end = find_line_end(it.data);
        regoff = 0;
        if (tmp_put_label_nextline) {
            tmp_put_label_nextline = false;
            pending_put_label = false;
            printd("newline put label..");

            pending_lable_ident = 0;
            pending_put_label = false;
            char *ret;
            asprintf(&ret, "__anonyn_%d", anonyn_index++);
            tmp_defer_rets = (str){ .data = ret, .len = strlen(ret) };

            int diff = s.code.count - tmp_to_resolve;

            s.code.data[tmp_to_resolve] |= ((diff) << 5); // TODO this only resolves imm19 for cbz, b.cond
            tmp_to_resolve = 0;
            macho_stab_loc(s.code.count * sizeof (u32), tmp_defer_rets);
            free(tmp_defer_rets.data);
            tmp_defer_rets = str_empty;
        }
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
        if (Next() != '{') {
            CompileErr("Syntax error: { expexted after the struct declaration");
        }
        Next(), c = Next();

        type_info structure = {
            .bsize = 0, .addr_type = ptype_not_addr, .name = token
        };
        ls_new_voidp(&structure.members, 8, "structure");

        while (c == ' ' || c == '\n') {
            c = Next();
        }
        if (c == '}') {
            printd("empty struct\n");
            goto brk;
        } else if (Is("    ")) {   // TODO need to accept one liner
loop_read_type:;
                type_info *t = read_type(&it, &c);
                if (t is NULL) {
                    CompileErr("Syntax error: invalid type: "), strprint(token), printd("\n");
                    goto loop;
                }
                structure.bsize += t->bsize;
                ls_add_voidp(&structure.members, t);
                TokenStart;
                ReadToken;
                TokenEnd;
                printd("name is '"), strprint_nl(token), printd("', ");
                c = Next();
                while (c is '\n' or c is ' ') {
                  c = Next();
                }
                if (c == '}') {
                  printd("end of struct\n");
                  goto brk;
                }
                printd("followed by...");
                goto loop_read_type;
        } else {
            CompileErr("Syntax error: indentation required after struct or '}' for empty struct.");
        }

    brk:

        ls_add_type_info(&types, structure);
        for (int i = 0; i < structure.members.count; ++i) {
            type_info *p = structure.members.data[i];
            printd(", %d: ", i), strprint_nl(p->name);
        }
        printd("\n");
        goto loop;
    }

    if (*token.data != '\n' && token.len != 1) {
        // printd("\ntoken '"), strprint_nl(token), printd("' (len: %zu, ident: %d, c: %c%d): ", token.len, ident, c, c);
        printd("\ntoken '"), strprint_nl(token), printd("': ");
    }

    if (str_equal_c(token, "ret")) {
        printd("ret\n");
        ls_add_u32(&s.code, RET);
        goto loop;
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
        printd("static label '"), strprint_nl(token); printd("', ");

        str nextsrc = (str){ it.data };

        for (int i = nextsrc.len; i < src.len; ++i) {
            if (Next() == '\n') {
                c = it.data[1];
                if (c != ' ' && c != '\n')
                    break;
            }
        }

        ++depth;
        printd("\n*** new stack, depth %d\n", depth);
        nextsrc.len = it.data - nextsrc.data;
        strprint(nextsrc);
        printd("end src\n");

        parse_scope(nextsrc, true, token);
        --depth;
        printd("\n*** endofrt\n");

        goto loop;
    }

    if (Is("(i32 Argc, addr addr c8 Argv)")) {
        printd("read args\n");
        if (main_defined) {
            CompileErr("Error: Arguments must be declared before main code\n");
        }
        add_nreg(&s, str_from_c("i32"), str_from_c("Argc"), 0, ptype_not_addr, 64, &named_reg_idx); // w0
        add_nreg(&s, str_from_c("c8"), str_from_c("Argv"), 1, ptype_addr_addr_addr, 64, &named_reg_idx); // x1
    }

    // if (!main_defined && token.len != 0 && depth == 0) {
    // }

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
        } else if (target_scratch){
            printd("target_scratch was: %d", target_scratch->reg);
            reg = target_scratch->reg;
            sf = nreg_sf(target_scratch);
        } else {
            printd("target_nreg was: null..");
            char *rewind = token.data - 1;
            while (IsSpace(*rewind)) {
                --rewind;
            }
            printd("last thing was.. %d(%c), ", *rewind, *rewind);
            if (*rewind is '>' && rewind[-1] isnt '>')
                reg = 0;
            else
                reg = 8;
            // else if (*rewind is ']')
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
                printd("target_nreg: %d, %d", target_nreg->reg, target_nreg->bsize);
            }
            ls_add_u32(&s.code, cmp(sf, reg, number, is_minus));

            c = Next();
            TokenStart;
            ReadToken;
            TokenEnd;

            if (!Is("->")) {
                CompileErr("Syntax Error: -> expected, but found %d", c);
                // CompileErr("next was %d", it.data[1]);
            }

            if (token.len > 0) {
                printd("branch to..");
                strprint(token);
                resolv tmp = { .name = token, .offset = s.code.count };
                ls_add_resolv(&resolves, tmp);
                ls_add_u32(&s.code, b_cond(0, cond));
            } else {
                printd("anonymous label..");
                printd("next: %d%d", it.data[0], it.data[1]);
                tmp_to_resolve = s.code.count;
                ls_add_u32(&s.code, b_cond(0, cflags_flip(cond)));    // tmp offset needs to be done later..
                pending_lable_ident = ident;
                pending_put_label = true;
            }

            if (it.data[0] is '\n') {
                if (pending_put_label)
                    pending_lable_ident += 4;
            } else if (pending_put_label) {
                tmp_put_label_nextline = true;
            }
        }
        printd("end\n");
        goto loop;
    }

    if (Is(">>")) {
        ls_add_u32(&s.jump_pair_start, s.code.count);
        ls_add_u32(&s.code, B);
        printd("jump pair");
        printd("\n");
    } else if (Is("<<")) {
        if (s.jump_pair_start.count == 0)
            CompileErr("Compile Error: no preceding >>");
        for (int i = 0; i < s.jump_pair_start.count; ++i) {
            u32 val = s.jump_pair_start.data[i];
            u32 diff = s.code.count - val;
            printd("jump to here, count was %d", s.code.count);
            printd("..diff was %d", diff);
            s.code.data[val] |= diff;
        }
        s.jump_pair_start.count = 0;
        printd("\n");
    }

    char tokc = token.data[0];
    if (tokc is '_' or IsAlpha(tokc)) {

        str name = token;

        if (Is(" :: ")) {
            printd(":: expression\n");

            nreg n = {
                .name = name, .bsize = 32, .is_addr = ptype_not_addr,
            };
            if (Is("stack")) {
                printd("type stack ");
                c = Next();
                n.bsize = 64;
                n.is_addr = ptype_stack_addr;
            } else if (Is("addr")) {
                printd("type addr ");
                c = Next();
                n.bsize = 64;
                n.is_addr = ptype_addr_addr;
            }
            TokenStart;
            ReadToken;
            TokenEnd;
            c = Next();
            type_info *t = type_find(token);
            if (t is NULL) {
                CompileErr("Compile error: invalid type: ");
                strprint_nl(token), printd(".");
                goto loop;
            }
            n.type = t;
            printd("type "), strprint_nl(t->name), printd(" ");
            printd("is_addr: %d\n", n.is_addr);
            if (n.is_addr == ptype_not_addr) {
                printd("not addr, size is %d\n", t->bsize);
                n.bsize = t->bsize;
            }

            nreg *find = nreg_find(&s.named_regs, token);
            n.reg = named_reg_idx;
            if (find != NULL) {
                n.reg = find->reg;
                *find = n;
                printd("found "), strprint(find->name);
            }
            if (n.reg > 28) {
                CompileErr("Error: used up all callee-saved registers\n");
            }

            if (find == NULL) {
                bool is_stack_alloc = memcmp(line_end - 2, "=[]", 3) == 0;
                // printd("line end was %c, is_stack_alloc %d..", line_end[-2], is_stack_alloc);

                if (is_stack_alloc) {
                    n.is_addr = ptype_stack_addr;
                }

                if (n.is_addr isnt ptype_stack_addr && !is_stack_alloc) {
                    // printd("inc reg idx..");
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

            fat f = { _objcode, objcode };
            if (find.find == NULL) {
                u32 symbolnum = stab_und.count;
                macho_stab_undef(token);
                printd("failed to find symbol, adding to undef");
                macho_relocent_undef(f, &s, symbolnum, ARM64_RELOC_BRANCH26, true);
            } else {
                macho_relocent(f, &s, find.symbolnum, ARM64_RELOC_BRANCH26, true);
                ls_add_int(&to_push_ext, relocents.count - 1);
            }
            // printd("find reloc '%p%s <%d>,", find.find, find.find, find.symbolnum);
            // printd("'\n");

            ls_add_u32(&s.code, BL);

            if (is_target_nreg(&it)) {
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
            printd("find reloc '%p%s <%d>,", find.find, find.find, find.symbolnum);
            printd("'\n");

            ls_add_u32(&s.code, B);

            if (is_target_nreg(&it)) {
                printd("target: %d", target_nreg->bsize);
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
        if (should_use_x0_reg(line_end, src, &it)) {
            reg = regoff;
        } else {
            char *iter = line_end + 2;
            while (*iter == ' ') {
                iter++;
            }
            while (IsToken(*iter)) {
                iter++;
            }
            if (iter[0] == '=' && iter[1] == '>') {
                reg = regoff;
            } else {
                reg = 8 + regoff;
            }
        }
    }

    if (*it.data is '&') {
        printd("alias..");
        c = Next();
        TokenStart;
        ReadToken;
        TokenEnd;
        strprint(token);
        nreg tmp = {
            .is_addr = false,
            .name = token,
            .reg = reg,
            .bsize = 32,
        };
        printd("reg was %d, %d", reg, regoff);
        ls_add_nreg(&s.scratch_aliases, tmp);
        token_consumed = true;
    }

    if (*it.data is '[') {
        printd("load...");
        c = Next();
        TokenStart;
        ReadToken;
        TokenEnd;


        printd("from "), strprint_nl(token);

        int reg2 = SP;
        int offset = 0;
        int size = 0;

        obj *target = obj_find(&s, token);
        if (target == NULL) {
            nreg *nreg_target = nreg_find(&s.named_regs, token);
            if (nreg_target != NULL) {
                if (c is ',') {
                    c = Next(), c = Next();
                    offset = read_int(token, &it);
                    printd("offset int was %d", offset);
                    c = *it.data;
                    if (c isnt ' ') {
                        CompileErr("Syntax Error: space expected");
                        goto loop;
                    }
                    c = Next();
                    if (Is("addr")) {
                        offset *= ADDRESS_SIZE;
                        size = ADDRESS_SIZE;
                    } else {
                        type_info *tinfo = read_type(&it, &c);
                        if (tinfo == NULL) {
                            CompileErr("Unknown type in offset expression");
                            goto loop;
                        }
                        printd("offset multiplied by %d", tinfo->bsize / 8);
                        offset *= (tinfo->bsize / 8);
                        size = tinfo->bsize / 8;
                    }
                } else {
                    size = nreg_target->bsize / 8;
                }
                printd("offset is %d", offset);

                reg2 = nreg_target->reg;
                
                printd("size: %d\n", size);

                printd("from nreg..end\n");
                token_consumed = true;
            } else {
                CompileErr("Error: unknown object or named register "), PrintErrStr(token);
                goto loop;
            }
        } else {
            size = target->size / 8;
            reg2 = SP;
            offset = target->offset;
        }

        c = Next();

        int dst_reg = reg;
        if (target_nreg != NULL) {
            if (it.data[0] == '\n') {
                printd("target: %d", target_nreg->reg);
                dst_reg = target_nreg->reg;
            }
        }

        u32 opc = ldr_size(dst_reg, reg2, offset, size);
        ls_add_u32(&s.code, opc);

        printd("next was %x, %c", c, c);
        printd("..end\n");
        token_consumed = true;
    }

    if (Is("=[")) {
        printd("store to ");
        str before = { token.data - 3, 2 };
        TokenStart;
        ReadToken;
        TokenEnd;
        strprint(token);

        if (token.len is 0) {
            size_t siz_bits = target_nreg->bsize;
            size_t siz_bytes = siz_bits / 8;
            // printd("new allocation.. from %d, %zu..", reg, siz_bits);
            printd("new allocation..");
            sf_t reg_sf = nreg_sf(target_nreg);
            size_t offset = s.stack_size - s.spaces_left;
            // printd("stack_size =0x%zx, spaces_left=0x%zx.., siz=0x%zx", s.stack_size, s.spaces_left, siz_bytes);
            u32 opc = str_imm(reg_sf, reg, SP, offset);
            ls_add_u32(&s.code, opc);
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

        nreg *find = nreg_find(&s.named_regs, token);
        if (find == NULL) {
            CompileErr("Error: unknown named register "), PrintErrStr(token);
        }
        sf_t reg_size = nreg_sf(find);
        if (find->is_addr == ptype_stack_addr) {

            obj *find = obj_find(&s, token);
            if (find == NULL) {
                CompileErr("Error: unknown named register "), PrintErrStr(token);
            }

            ls_add_u32(&s.code, str_imm(reg_size, reg, SP, find->offset));
        } else if (find->is_addr == ptype_addr_addr) {
            ls_add_u32(&s.code, str_imm(reg_size, reg, find->reg, 0));
        } else {
            CompileErr("Error: trying to store to non-address register\n");
        }
        c = Next();
        goto loop;
    }
// arithmetics
    addsub add_or_sub = ADDSUB_NONE;
    if (tokc is '+')
        add_or_sub = ADDSUB_ADD;
    else if (tokc is '-')
        add_or_sub = ADDSUB_SUB;

    if (add_or_sub isnt ADDSUB_NONE) {
        c = Next();
        if (tokc is '+')
            printd("add..");
        else
            printd("sub..");
        u8 reg1 = 8, reg2 = 9;
        if (s.code.count == 0) {
            reg1 = 0, reg2 = 1;
        }
        u32 opc = add_shft_f(W, add_or_sub, reg, reg1, reg2, ASH_LSL, 0);
        ls_add_u32(&s.code, opc);
        token_consumed = true;
        printd("\n");
    }
    if (tokc is '*') {
        c = Next();
        u8 reg1 = 8, reg2 = 9;
        ls_add_u32(&s.code, mul(W, 8, reg1, reg2));
        token_consumed = true;
    } else if (tokc is '/') {
        c = Next();
        u8 reg1 = 8, reg2 = 9;
        ls_add_u32(&s.code, sdiv(W, 8, reg1, reg2));
        token_consumed = true;
    }

// literals
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
        printd("%ld to r%d", number, reg);
    }
    if (tokc is '{') {
        printd("start of struct literal..");
        c = Next();
        c = Next();
        type_info *t = target_nreg->type;
        if (t->bsize <= 64) {
            u32 offset = 0;
            for (int i = 0; i < t->members.count; ++i) {
                type_info *u = t->members.data[i];
                int n = read_int(token, &it);
                printd("<%d>, ", n);
                if (i < t->members.count - 1 && *it.data != ',')
                    CompileErr("Syntax Error: Comma expected at struct literal");
                c = Next();
                c = Next();
                // TODO stub impl just for point, with two i32s, next we'll do stub impl for str and generalize.. may need to refer to aapcs
                if (offset == 0) {
                    ls_add_u32(&s.code, mov(X, reg, n));
                } else {
                    ls_add_u32(&s.code, movk_imm(X, reg, n, 32));
                }
                offset += u->bsize;
            }
        } else {
            CompileErr("Error: struct literal not implemented for >64 bits\n");
        }
    }

    if (IsAlpha(tokc)) {
        nreg *find = nreg_find(&s.named_regs, token);
        if (find is NULL) {
            target_scratch = nreg_find(&s.scratch_aliases, token);
            if (target_scratch is NULL) {
                CompileErr("Error: unknown named or scratch alias register(mov) "), PrintErrStr(token);
            } else {
                goto loop;
            }
        }

        if (find != NULL) {
            if (find->is_addr == ptype_stack_addr) {
                obj *find = obj_find(&s, token);
                ls_add_u32(&s.code, add_imm(X, reg, SP, find->offset));
            } else {
                sf_t sf = nreg_sf(find);
                ls_add_u32(&s.code, mov_reg(sf, reg, find->reg));
            }
            token_consumed = true;
        } else {
            CompileErr("Error: unknown named register(mov) "), PrintErrStr(token);
        }
    } else if (tokc is '"') {
        // is going to be a string
        c = Next();
        TokenStart
        while (c != '"' && c != '\0') { c = Next(); }
        TokenEnd

        printd("original: `"), strprint_nl(token), printd("` ");
        for (int i = 0; i < token.len; ++i) {
            char d = token.data[i];
            if (d != '\\')
                continue;
            d = token.data[i + 1];
            printd("(has escape seq %c) ", d);
            if (d == 'n') {
                token.data[i] = '\n';
                memmove(token.data + i + 1, token.data + i + 2, token.len - i);
                token.len -= 1;
            }
        }
        printd("str lit: `"), strprint_nl(token), printd("`");

        macho_stab_stringlit();

        ls_addran_char(&strings, token.data, token.len);
        if (it.data[1] == '0') {
            printd("is null terminated string");
            c = Next();
            ls_add_char(&strings, '\0');
        }
        c = Next();
        printd("\n");

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
        printd(", ");
        token_consumed = true;
        goto loop;
    }

    if (token.len == 0 && (c == '\n' || c == '\0'))
        token_consumed = true;
    if (!token_consumed && !(c == '\n' && it.data[1] == '\0'))
        CompileErr("token is not consumed %d", c), strprint(token), printd("\n");
    if (c == '\n') {
        // printd("\\n\n");
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
        // printd("stack size: 0x%zx", s.stack_size);
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

    fat f = {_objcode, objcode};
    macho_stab_ext(f, name);

    // if (!main_defined && depth == 0) {
    //     printd("main defined here at write position\n");
    //     macho_stab_ext(f, str_from_c("_main"));
    //     main_defined = true;
    // }

    usize prologue_len = s.prologue.count;
    if (s.stack_size > 0) {
        u32 prologue_sp = sub(SP, SP, s.stack_size);
        write_buf(&objcode, &prologue_sp, sizeof (prologue_sp));
        ++prologue_len;
        ls_add_u32(&s.epilogue, add_imm(X, SP, SP, s.stack_size));
    }

    ls_add_u32(&s.epilogue, RET);

    write_buf(&objcode, s.prologue.data, s.prologue.count * sizeof(u32));


    for (int i = 0; i < s.relocents.count; ++i) {
        u32 index = s.relocents.data[i];
        relocents.data[index].r_address += fat_len((fat){_objcode, objcode}) * sizeof(u32);
        printd("index: %u, r_address: 0x%x\n", index, relocents.data[index].r_address);
    }

    write_buf(&objcode, s.code.data, s.code.count * sizeof(u32));
    write_buf(&objcode, s.epilogue.data, s.epilogue.count * sizeof(u32));

    if (is_routine) {
        const long offset = (objcode - (void *)_objcode) - (prologue_len * sizeof (u32));
        for (int i = 0; i < to_push.count; ++i) {
            int index = to_push.data[i];
            stab_loc.data[index].n_value += offset;
        }
        for (int i = 0; i < stab_loc.count; ++i) {
            stab_loc.data[i].n_value += prologue_len * sizeof (u32);
        }
    }

    stack_context_free(&s);
    printd("\nparse end\n");
}

