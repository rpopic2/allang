#pragma once
#include "error.h"
#include "parser_util.h"
#include "slice.h"
#include "list.h"
#include "stack_context.h"
#include "str.h"
#include "typedefs.h"

typedef struct {
    i32 bsize;
    ptype addr_type;
    str name;
    ls_voidp members;   // actually list of ls_type_info *
} type_info;

ls (type_info);

ls_type_info types;

void types_init(void) {
    ls_new_type_info(&types, 256, "all types");
// builtin types

    type_info c8 = {
        .members = NULL,
        .bsize = 8,
        .addr_type = ptype_not_addr,
        .name = str_from_c("c8"),
    };
    ls_add_type_info(&types, c8);

    type_info i32 = {
        .members = NULL,
        .bsize = 32,
        .addr_type = ptype_not_addr,
        .name = str_from_c("i32"),
    };
    ls_add_type_info(&types, i32);

    type_info i64 = {
        .members = NULL,
        .bsize = 64,
        .addr_type = ptype_not_addr,
        .name = str_from_c("i64"),
    };
    ls_add_type_info(&types, i64);

    // type_info tmp = {
    //     .size = 32,
    //     .addr_type = ptype_not_addr,
    //     .name = str_from_c("point"),
    // };
    // ls_new_voidp(&tmp.members, 8, "struct point");
    // ls_add_voidp(&tmp.members, &types.data[0]);
}
void types_destroy(void) {
    for (int i = 0; i < types.count; ++i) {
        ls_delete_voidp(&types.data[i].members);
    }

    ls_delete_type_info(&types);
}

type_info *type_find(str name) {
    for (int i = 0; i < types.count; ++i) {
        if (str_equal(types.data[i].name, name)) {
            return types.data + i;
        }
    }
    return NULL;
}

type_info *read_type(str_iter *it, int *c) {
    printd("read type\n");
    // str_iter it = *pit;
    // int c = *pc;
    type_info tmp_info = {

    };
    if (_if_is("stack", it, c)) {
        printd("type isstack ");
        *c = iter_next(it);
        tmp_info.bsize = 64;
        tmp_info.addr_type = ptype_stack_addr;
    } else if (_if_is("addr", it, c)) {
        printd("type is addr ");
        *c = iter_next(it);
        tmp_info.bsize = 64;
        tmp_info.addr_type = ptype_addr_addr;
    }

    str name = { .data = it->data };
    while (*it->data != ' ' && *it->data != '\n' && *it->data != '\0' && *it->data != ')') {
        ++it->data;
    };
    name.len = it->data - name.data;

    type_info *find = type_find(name);
    if (find == NULL) {
        return NULL;
    }
    if (tmp_info.addr_type != ptype_not_addr) {
      find->addr_type = tmp_info.addr_type;
      find->bsize = tmp_info.bsize;
    }
    return find;

    // if (_if_is("i64", it, c)) {
    //     printd("type i64 ");
    //     *c = iter_next(it);
    //     if (!info.addr_type)
    //         info.size = 64;
    // } else if (_if_is("i32", it, c)) {
    //     printd("type i32 ");
    //     *c = iter_next(it);
    //     if (!info.addr_type)
    //         info.size = 32;
    // } else {
    //     char tmp = *it->data;
    //     printd("%c(%d)", tmp, tmp);
    // }
    // // *pit = it;
    // // *pc = c;
    // return info;
}

