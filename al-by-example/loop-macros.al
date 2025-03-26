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
        ~ ->
        i ? condition ge<-
        1 +=init

#macro foreach (decl..collection)
    i :: 0; i < collection.len; ++i; @for
        decl :: i, collection @at
