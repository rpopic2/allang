fn main:
    0 -> i
    loop:
        "%", i -> fmt -> print
        i, 1 -> + -> i
        < 10 ? loop
    ret

// https://zig.guide/language-basics/while-loops

|u8 i| :: 2
i < 100 @while
    2 *=i

// or you could...

2'u8
< 100 @while
    2 *=

//

|u8 sum| :: 0
|u8 i| :: 1
i <= 10, i += 1 @while.zig
    i +=sum

//

|u8 sum| :: 0
|u8 i| :: 0
i < 3, i += 1 @while.zig
    i is 2 ->continue
    i +=sum

//

|u8 sum| :: 0
|u8 i| :: 0
i < 3, i += 1 @while.zig
    i is 2 <-
    i +=sum

// https://zig.guide/language-basics/labelled-loops

count :: 0'usize
outer: i :: 1..8 + 1 @range
    j :: 1..5 + 1 @range
        1 +=count
        ->outer.continue

// https://zig.guide/language-basics/loops-as-expressions
rangeHasNumber: (usiz begin, usiz end, usiz number =>b8)
    i :: begin
    i < end, i += 1 @zig.while
        i is number -> true ret
    false ret

"Hello" print=>, 3 @loop.unroll
