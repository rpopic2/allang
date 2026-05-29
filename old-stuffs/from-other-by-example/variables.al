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

// https://zig.guide/language-basics/assignment

|i32| constant :: 5
u32 variable :: 5000

i32 constant :: 5
|u32| variable :: 5000

constant :: 5
|variable :: 5000
|addr c8 ptr ::
addr |c8 ptr ::
|addr |c8 ptr ::

foo: (|addr c8 =>|cmd)

i32 constant :: 5
u32 var variable :: 5000

// about overflow

// wrapping operaters are cool...
[a] +? 1 overlow-> panic
=[]

// https://odin-lang.org/docs/overview/#variable-declarations

stack i32 x // okay, but you cannot use before assignment
stack i32 y, z

// i32 p   // just declares it, but compile error unless it's a routine parameter!
i32 i := 3
j := i32 3
k := i

[p] + 3 ::[]
3 +::i
2 ::p

i << 3 + i
i <<+ 3

3 +::i
3 ::i
// ++ no i++!
// but..
[p, i32 3, +=p] // is possible
[+=p, i32 3]
[p, 33]!
[p], 33

// or maybe even..?

i :: 0
j :: i
j :: j + 2
j :: 2 + // is the shorthand needed..?
j +: 2
i += j
++i
j :: 3

f :: 3.0
f +: 'c'    // cannot add char to float!
f :: 'c'    // ok!
f +: 1.2    // cannot add float to char!

// maybe?

i :: 0
j :: i
j + 2 =
j + 2 =j
i + j =
++i
3 =j

j + 1 =
[i] + 1 =[]


// original

i :: 0
j :: i
2 +=j
2 + j =j
j +=i
++i
3 =j

// middle
i :: 0
j :: i
1 +>i
2 + j >j
++i
3 >j

// practical..?

i := 0
j := i
1 +>i
2 + j >j
i += j
++i
j = 3

// c

i := 0
j := i
i += 1
j += 2
i += j
++i

//

x :: 10
x :: 20
y, z :: 20, 30
test, z :: 20, 30    // ok, it shadows it. for register, it will just use the same register.. which effectively just assigns it!

x :: i32 123
x :: 637

x, y :: 1, "hello"
y, x :: "bye", 5

i32 x :: 123
x :: 123

