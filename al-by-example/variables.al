// https://gobyexample.com/variables

#alias std.io.print
#alias std.str

str a :: "initial"
a print=>

i32 b :: 1
i32 c :: 2

b" "c print=>

d :: true

i32 e :: 0
e print=>

// or
[i32 e]= :: i32   // going to contain some garbage value
[e] print=>
e print=>   // prints out address

f :: "apple"
f print=>

// https://doc.rust-lang.org/stable/rust-by-example/variable_bindings.html

|immutable_binding| :: 1
mutable_binding :: 1

"Before mutaion: "mutable_binding print=>

1 +=mutable_binding

"After mutaion: "mutable_binding print=>

// 1 +=immutable_binding // compile err!



// # scope and shadowing

long_lived_binding :: 1
_:
    short_lived_binding :: 2
    "inner short: "short_lived_binding print=>

// "outer short: "short_lived_binding print=> // compile err! cannot find short_lived_binding
"outer long: "long_lived_binding print=> // compile err! cannot find short_lived_binding

// but unlike rust, if you had a named label (or a scope)

named:
    short_lived_named :: 2

named.short_lived_named print=> // you can access it!


// shadowing

shadowed_binding :: 1

_:
    "before being shadowed: "shadowed_binding print=>
    shadowed_binding :: "abc"
    "shadowed in inner block : "shadowed_binding print=>
    "but you can access : "main.shadowed_binding print=>


shadowed_binding :: 2
"shadowed in outer block: "shadowed_binding print=>
// this time you cannot access the first variable.



// # declare first

// a_binding ::    // this is not allowed for registers
[a_binding]= ::     // you can declare stack variables first
_:
    x :: 2
    x * x =[a_binding]

"a binding: "[a_binding] print=>

[another_binding]= ::

// "another binding "another_binding   // compile err! not init!

// if you wish it to be not zeroing out the stack... do it explicitly
[another_binding]= :: i32 5

1 =[another_binding]

// blocks as expression?

x :: 5'u32


y ::
    x_squared :: x * x
    x_cube :: x_squared * x
    x_cube + x_squared + x

z ::
    2 * x
