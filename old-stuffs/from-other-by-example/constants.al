// https://gobyexample.com/constants

std.
#alias .str
#alias .io.print
#alias .math

|s| :: "constant"

s print=>

|n| :: 500000000    // i32

|i128 d| :: 3e20 @/ n // would be i128 divided by i128 zero-extended from i32
d print=>
// the value of d is i32, rounded

d to i64 print=>

n math.sin=> print=>

|addr i32| b :: i32 std.heap.alloc=>    // const address of const i32
|addr| i32 b :: i32 std.heap.alloc=>    // const address of mutable i32
addr |i32| b :: i32 std.heap.alloc=>    // mutable address of const i32
|b|             // makes it const address of const i32
// |addr| i32 b    // it would make it const address to mutable i32.. but you cannot make it mutable if you made it const already!!

// https://doc.rust-lang.org/stable/rust-by-example/custom_types/constants.html

// putting bars around it makes it const. you cannot change where language points, and how long it is.

|language| :: "allang"    // the actual string lives at the code section, language being just a pointer to it. they have static lifetime, becaue main is not a function, but just an entry point.
THRESHOLD: 10   // this is just embedded with the code, so you can never change it, which is const

is_big(i32 n =>bool)
    n > THRESHOLD

n :: 16
"This is " language print=>
"The threshold is " THRESHOLD print=>

n is_big=> ? "big" : "small" %
n" is "% print=>

// 5 =THRESHOLD    // illeagal!
// 5 =[THRESHOLD]

// https://odin-lang.org/docs/overview/#constant-declarations

x: "what"

y: i32 123
i32 y: 123
i32 z: y + 7    // it's comptime

t :: y + 7      // it's not comptime
t: y + 7        // now it's comptime
