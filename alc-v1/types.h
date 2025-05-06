#pragma once
#include "error.h"
#include "parser_util.h"
#include "slice.h"
#include "stack_context.h"
#include "typedefs.h"

typedef struct s_type_info {
    i32 size;
    ptype addr_type;
} type_info;

type_info read_type(str_iter *it, int *c) {
    printd("read type\n");
    // str_iter it = *pit;
    // int c = *pc;
    type_info n = {

    };
    if (_if_is("stack", it, c)) {
        printd("type stack ");
        *c = iter_next(it);
        n.size = 64;
        n.addr_type = stack_addr;
    } else if (_if_is("addr", it, c)) {
        printd("type addr ");
        *c = iter_next(it);
        n.size = 64;
        n.addr_type = addr_addr;
    }
    if (_if_is("i64", it, c)) {
        printd("type i64 ");
        *c = iter_next(it);
        if (!n.addr_type)
            n.size = 64;
    } else if (_if_is("i32", it, c)) {
        printd("type i32 ");
        *c = iter_next(it);
        if (!n.addr_type)
            n.size = 32;
    } else {
        char tmp = *it->data;
        printd("%c(%d)", tmp, tmp);
    }
    // *pit = it;
    // *pc = c;
    return n;
}

