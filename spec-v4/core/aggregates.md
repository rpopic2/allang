# declaring structs

struct foo {  // curly braces for structs
    i32 X,      // members delemted by comma.
    addr i32 P, // paddings are inserted automatically,
                // and memory structure will obey the order of declaration
}               // now struct point has been added to the type system

// struct name should start with lower case
// imported struct names also will be in lower case


# struct literals

point <{ 1, 2 }>
point [ 1, 2 ]  // loads address of struct in text section
                // note that this is immutable
P :: point [ 2, 3 ] // p points to const point. p is immutable.
Q :: point [ 2, 3 ] @copyto =[] // now copied on stack
R ::
    #sizeof point# bytes std:heap:alloc=> This()
    point < 2, 3 > @copyto This  // now copied on the heap

S :: < 2, 3 > =[] @point.new                // or use constructor macros
T :: < 2, 3 > std.heap.alloc @point.new

U :: point < y= 3 >  // y is 3 and others are zero

# array literals

i32 < 1, 2, 3 > // loads address and length of array in text section
                // note that this is immutable

Arr :: i32 < 1, 2 .. 100= 3 >   // fill 2 until 100th element
Arr.Len print=>                 // prints out 101


# string literals

"hello world"   // loads address and length of string in text section
                // note that this is immutable

""EOF           // empty strig followed by any word makes a heredoc
    this is multiple line and \not escaped
    type whatever you want even "quote" marks
    keep indented one depth deeper
    and indentation before this is ignored
        but this indentation is recognised
EOF             // end heredoc


# zero aggreates

3 i32 []        // zero array literal. zero arrays and structs are optimised out, so no load address is performed here
foo {}          // zero struct


# structuring and destructuring

point P :: .... // makes point p
X, Y :: P       // destructures p and loads x and y into registers
point Q :: { X, Y } // structures it back

# unions

union u {
    i32 A,
    u64 B,
}

U :: u { i32= 3 }
V :: u { u64= 3 }

# enum unions

enum union result {
    void Error
    i32 Ok
}

R :: result { Error }
T :: result { Ok, 3 }

R is { Error } ->
    "error occured" print=>
T is { Ok, &Value } ->
    `value was `Value print=>

