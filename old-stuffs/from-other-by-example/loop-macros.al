// macros

@inline loop:
    :
        continue:
        ~ ->.

@inline while: (@code condition)
    @loop
        condition @std.bool.invert <-

@inline for: (@code init, @code condition, @code defer)
    :
        init
    continue:
        ~ ->.
        condition @std.bool.invert <-
        ~ defer

@inline times: (@reg reg, i32 times)
    reg :: 0, reg < times, times += 1 @for

@inline range: (@decl.int init..@code condition)
    :
        init
    continue:
        ~ ->.
        i >= condition <-
        1 +=init

#macro foreach (decl..collection)
    i :: 0; i < collection.len; ++i; @for
        decl :: i, collection @at

@inline foreach (@slice s[@reg r])
    i :: 0..s.len @for
        __i :: 0
    continue:
        #block.append ++__i; ->continue
        __i >= s.len <-
        r :: s[__i] @at


