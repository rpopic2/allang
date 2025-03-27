// assignment happens from right to left <-

// reg

a :: 1  // named reg a has 1 assigned, defaulting to i32

a :: 2  // named reg a has 2 assigned. technically it's a shadowing, but after the shadow, a is not used anymore, so it assigns to the same register again, which is just an assignment!

a :: 3.0    // shadows the a declared before! although it may look like a duck typing, it is actually checked statically

a +: 2.1    // add the value before and shadow it again

a +: 'c'    // error! you cannot assign u8 to float!

a :: i32 3 // you can annotate type to go explicit..


// stack

b :: 1 =[]  // technically not an assignemnt. it just store 1 to a compile-time stack offset. b now holds offset address from stack pointer, though you can use b like a pointer and there is no semantic difference.




// heap

c~ :: 4 heap.alloc.bytes=>   // what gets assigned to c is the rightmost expression! it allocates 4 bytes on the heap and return ptr to it. ~ notation just means that it owns it.

// fn parameters

foo: (i i32)    // function parameters are registers. they are not named registers, so they don't get :: syntax.

