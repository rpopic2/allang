// https://gobyexample.com/slices

s :: str ||
`uninit: `s` `s.data isnt pointing` `s.len is 0 print=>

s :: 3 @str.new ~~> str.delete=>
`emp:`s` len:`s.len` print=>

'a' =[s.data.0]
'b' =[s.data.1]
'c' =[s.data.2]
`set: `s print=>
`get: `[s.data.2] print=>
`len: `s.len print=>

"d", s .append=> s()
"e", "f", s .append=> s()
`apd: `s print=>

c :: s.len @str.new =[] ~~> str.delete
s..c str.copy=>

l :: s, 2..5 str.slice=>
`sl1: `l print=>

l :: s, 0..5 str.slice=>
`sl2: `l print=>

l :: s, 2.. str.slice=>

t :: { 'g', 'h', 'i' } @str.from
t2 :: { 'g', 'h', 'i' } @str.from
t, t2 str.equal=> is true->
    "t == t2" print=>

twoD :: 3 i32 i32 || =[]
i :: 0..3 @range
    inner_len :: i + 1
    j :: 0..inner_len @range
        twoD[i] @at
        [j] @at <: i + j
twoD print=>

// mine

hello: "Hello World!"
slice :: 6..11 hello str.slice=>

slice print=> // prints out "World!"


// more in arrays.al

// https://zig.guide/language-basics/slices

total: (slice u8 values =>usiz)
    |usize sum| :: 0
    values, v @foreach
        sum += v
    sum

test "slices"
    array: u8 { 1, 2, 3, 4, 5 }
    slice :: array, 0..3 @slice
    slice total=>
    expect is 6

test "slices 2"
    array: u8 { 1, 2, 3, 4, 5 }
    slice :: array, 0..3 @slice
    expect #typeof slice is slice u8

test "slices 3"
    array: u8 { 1, 2, 3, 4, 5 }
    slice :: array, 0.. @slice


// http://odin-lang.org/docs/overview/#string-iteration
x :: "ABC"

c8 codepoint, i32 index @c8.foreach.index
    index` `codepoint print=>

index :: 0..x.len @range
    [x, index c8] print=>

c8 addr // type of c-string.

str :: "Hello"
cstr :: "Hello"0.data

cstr2 :: str { cstr, cstr libc.strlen=> }
nstr :: str.len
ncstr :: cstr libc.strlen=>

// http://odin-lang.org/docs/overview/#rationale-behind-explicit-overloading

fibonaccis: 6 int | 0, 1, 1, 2, 3, 5 |
slice i32 s :: fibonaccis 1..4 @slice
s print=>

s.len

| 1, 6, 3 |
[ 1, 6, 3 ]

a: 6 i32
a 0..6 @slice
a ..6 @slice
a 0.. @slice
a .. @slice

s :: slice i32 || // nil slices
s.data isnt pointing ->
    "s is nil!" print=>

// dynamic arr
x :: std.list.new=> ~>
~ x ~> std.list.delete

123, x .append=>
4, 1, 74, 3, x .append=>
x clear=>
