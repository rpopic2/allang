struct inner {
    A i32, B i32
}
inner:print: addr inner, Sink addr sink =>
    inner.A] .print Sink =>
    inner.B] .print Sink =>

struct outer {
    @inner, C i32
}


O :: outer{.A 3 .B 3 .C 8}

O.inner .print {.To std:Out} =>

I :: O.inner

struct outer2 {
    inner, C i32
}

[Iter]++
[Iter++]

Iter[]++
Iter++[]

tokenize: addr bi_iter u8 => slice? u8
    Iter :: [Self]
    defer Iter =[Self]

    Iter[]++ ? eret
    is ' ' ? ret <[Self.Current]..

