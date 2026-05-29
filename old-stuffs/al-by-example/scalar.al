// al is a strictly and statically typed language
// types are inferred.

i :: 0 //i defaults to i32. literal '0' is not always i32!
j :: i64 1 // i64. i64 1 is syntax for i64 literal.
d :: usize 234
d :: 0.3 // defaults to f64. dot is mandatory.
f :: f32 0.2
b :: true
c :: 'c' // c8. also known as utf-8 rune.
s :: "hello" // slice.c8

// to cast between ints,
i :: i32 256
b :: i8 i   // becomes 255
x :: i64 b  // widening
u :: u8 b  // becomse -1
// to cast to/from floats
f :: f64 3.0
i :: f round to i32
i :: f ceil to i32
i :: f floor to i32
d :: #f64.from i
// to reinterpret. if you are coming form rust, be careful!
reinterpret :: f as i32

i : 3.0 // error!

// type addr is just type checked usiz
ptr~ :: 4 @std.heap.alloc // owned address. addr~ i32, more on memory-safety.al
p :: ptr // unowned address, copied. addr i32
// pointers are called address in al.
p2 :: addr 2345  // address literals
p2 :: stack 4   // stack offset literals
ptr :: not pointing // nullptr


// to give it an explicit type, put it on the left (like C). we recommend not explicitly writing type on the left. almost everything can be declared without type on the left.

i32 i :: 0

