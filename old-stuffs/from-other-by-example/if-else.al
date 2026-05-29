// https://gobyexample.com/if-else

#alias std.io.print

// modulo it is actually a macro..! it does whatever optimizations possible
// calculating modulo fast is not done simple
7, 2 @mod.eq 0 -> // it would be 7 % 2 == 0 in c.
    "7 is even" print=>>    // =>> it jumps to next << after returning
:
    "7 is odd" print=>>

7, 2 @mod.eq 0 ->
    "7 is even" print=>,
: "7 is odd" print=>

8, 4 @mod.eq ->
    "8 is divisible by 4" print=>

(8, 2 @mod.eq 0) -> (7, 2 @%.eq 0) -> // chains it.
    "either 8 or 7 are even" print=>

num :: 9
num < 0 ->
    "is negative" print=>>
or num < 10 ->
    "has 1 digit" print=>>
:
    "has multiple digits" print=>>
<<

// but you could do it better!!! this is faster and more maintainable
num :: 9
num
    < 0 -> "is negative" >>
    < 10 -> "has 1 digit" >>
    -> "has multipl digits" >>
<<
print=>

num :: 9
num
    < 0 -> "is negative" >>
    < 10 -> "has 1 digit" >>
    : "has multipl digits"
<<
print=>


7, 2 @%.eq 0 ->
    "7 is even" >>
:
    "7 is odd" >>
<<
print.ln=>

7 @/ 4 // we also have a macro for division


// https://doc.rust-lang.org/stable/rust-by-example/flow_control/if_else.html

// 1. it's a assembly like way of doing it. not recommened unless you are hacking
n :: 5
// ? is a cmp (compare) operator.
n < 0 ->
    n" is negative"
n > 0 ->
    n" is positive"
: n" is zero"
print.nl=>  // it's a print with no linebreak! not a typo

// 2. more readable way
n < 0 ->
    //...

// 3. better way
n ? 0   // just pin it on w8 register
< ->
> ->
->

big_n ::
    n < 10 and n > -10 ->
        ", and is a small number, increase ten-fold" print=>
        10 * n <- // this operator breaks and escapes current scope
    :
        ", and is a big number, halve the number" print=>
        n / 2 <-

n" -> "big_n println=>

// https://zig.guide/language-basics/if

a :: true
|u16 x| :: 0

a is true ->
    1 +=x
:
    2 +=x

// we do have a ternary operator, but there can be only values on each side of it. no expression allowed.

a :: true
|u16 x| :: 0
a ? 1 : 2
+=x


// https://odin-lang.org/docs/overview/#if-statement

x >= 0 ->
    `x is positive` print=>

:
    x :: foo=>
    x < 0 ->
    `x is neagative` print=>

foo=>
is < 0 -> `x is negative` print=>,


foo=>
is < 0 -> `x is negative` print=>,
is 0 -> `x is zero` print=>,
: `x is positive` print=>,

// might be better...
    foo=> ? 0
        lt-> `x is negative`
        eq-> `x is zero`
        : `x is positive`
    print=>

architectures.
arch :: ONIN_ARCH
is .i386, .wasm32, .arm32 ->
    "32 bit",
or .amd64, .wasm64p32, .arm64, .riscv64 ->
    "64 bit",
or Unknown
    "Unknown arch"

i
is 0 -> do sth
: foo=>

c
is 'A'..++'Z' @in_range,
else 'a'..++'z' @in_range,
else '0'..++'9' @in_range ->
    "c is alphanumeric" print=>

x
is 0..10 @in_range ->
    "units"
else 10..13 @in_range ->
    "pre-teens"
else 13..20 @in_range ->
    "teens"
else 20..30 @in_range ->
    "twenties"
: "out of range!" panic

switch_table: 30 str {
    0: "units"..,
    10: "pre-teens"..,
    13: "teens"..,
    20: "twenties"..
}
// it's just a 30 fat pointers!

str[x] @at is Error
    "out of range!" panic
.Ok print=>

Foo:
    #enum {
        A, B, C, D
    }

f :: Foo.A
is .A -> "A"
,is .B -> "B"
,is .C -> "C"
,is .D -> "D"
: "?"
print=> // to use this, the switch above should be exhaustive

f
is .A -> "A" print=>
is .D -> "D" print=>
// now you can match partially because the matched value is not used after it.

Foo:
    @union {
        i32 i, b8 b
    }

f :: Foo { 123 }
is i -> "int"
or b -> "bool"
: ""
print=>

i
is 0 -> foo=>
is 1 -> bar=>

i
is 0 -> foo=>
else 1 -> bar=>



