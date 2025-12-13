# load and stores

[A]             // loads from the address which register a is pointing at
0 =[B]          // stores 0 to the address which register b is pointing at

[A, 3 i32]      // offsets 3 * 4 bytes. note that this is not bound checked
A[3]            // this is a bound checked version

[Point.X]       // offsets by offset of struct member
[[A].B]          // nested

# iter types
// iter types can change where itself it pointing to.

[A(+= 3 i32)]   // adds 3 * 4 bytes to a and loads
\[A](+= 3 i32)  // loads and add 3 * 4 bytes to a

