// https://doc.rust-lang.org/stable/rust-by-example/primitives.html

#alias println std.print.ln

b8 logical :: true

f64 a_float :: 1.0
an_integer :: 5'i32

default_float :: 3.0    // f32
default_integer :: 7    // i32

interred_type :: 12
// 4'294'967'296'i64 =inferred_type // inferring from other line..? don't know we'll impl it for now

mutable :: 12
21 =mutable

// true =mutable // compile err! the type is different!

mutable :: true   // shadowing is allowed

i32 5 my_array :: { 1, 2, 3, 4, 5 }
my_tuple :: { 5'u32, 1'u8, true, -5.04'f32 }
0x12
0b12
0o12

-2.5e-3

// example below are compile errors, but you can see how std.print.ln is used.
// true && false #.to_str=> %
// "true AND false is ", % println=>

// true || false #.to_str=> %
// "true OR false is ", % println=>

// true! #.to_str => %
// "NOT true is ", % println=>

