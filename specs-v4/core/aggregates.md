# declaring structs

struct foo {  // curly braces for structs
    i32 X,      // members delemted by comma.
    addr i32 P, // paddings are inserted automatically,
                // and memory structure will obey the order of declaration
}               // now struct point has been added to the type system

// struct name should start with lower case
// imported struct names will be changed to lower case

structs with size less than or equal to 16 bytes are called short structs. they can be placed in registers, while others cannot.

# struct literals

struct point {X i32, Y i32}

P :: point{.X 1 .Y 2}  // creates struct in a register
Q :: point<.X 2 .Y 3> // p points to const point. p is immutable.
R :: point<.X 2 .Y 3> @copyto =[] // now copied on stack

S ::
    &Heap .alloc point .size @ bytes =>
    point{.X 2, .Y 3} =[] // copied to the heap
T :: &Heap .new point{2, 3} // or use the constructor macro

v :: point{.Y 3 .. 0}  // y gets 3, others members zeroed out

it is better to use short arrays for points,

struct point{2*i32}
P :: point{1, 2}
P.X, P.0 // equivalant syntax

# array literals

*i32<1, 2, 3> // loads address and length of array in text section
                // note that this is immutable

Arr :: *i32<1, 2 .. 2 .100 3>   // fill 2 until 100th element
Arr .Len @ print=>                 // prints out 101


# string literals

"hello world"   // loads address and length of string in text section
                // note that this is immutable

""EOF           // empty strig followed by any word makes a heredoc
    this is multi-line literal and it is \not escaped.
    type whatever you want even "quote" marks
    keep this indented
    one level of indentation before this is ignored
        but this indentation is part of this literal
EOF             // use the same marker to end heredoc


# zero aggreates

3*i32{.. 0}        // zero array literal. zero arrays and structs are optimised out, so no load address is performed here
foo{.. 0}          // zero struct


# structuring and destructuring

P :: point{1, 2}  // make point p
X, Y :: (P)       // destructure p into x and y
Q :: point{X, Y}  // structure it back

# unions

union u {
    i32 A,
    u64 B,
}

U :: u{.i32 3}
V :: u{.u64 3}

# enum unions

enum union result {
    void Error
    i32 Ok
}

R :: result {.Error}
T :: result {.Ok 3}

R is {.Error} ->
    "error occured" print=>
T is {.Ok %Value} ->
    `value was `Value print=>


# copying structs

[Large_Struct]  // error! structs larger than 16 bytes cannot be loaded
Large_Struct memcpy Dest   // now copies the struct

# struct equality

Large_Struct memeq Dest   // compares all memory
Large_Struct .is Dest @  // or use user-defined macros

