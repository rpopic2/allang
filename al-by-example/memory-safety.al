// first of all, memory is manually allocated.
// programmer is responsible of managing memory...
// but al can help programmers doing that, and it is much more easier than C!
// address is used for pointer, but you can use it interchangably

// we have two versions of linked list to demonstrate this memory safety in real example
// check out linked-list.al

// # a owned pointer and unowned pointer

// # owned address

ptr :: 4 std.heap.alloc_bytes=> // this alloc_bytes routine returns an owning addr(addr~, ~ denotes owning pointer). you will have to either free this addr or move ownership, within this scope, or else compiler will complain.
// if you are familliar with c++, all pointers are unique_ptr by default!

// ptr :: 4 std.heap.alloc_bytes=> // syntax error! alloc_bytes returns owned address. '~' is missing!

ptr std.heap.free=>   // moves ownership, as free takes addr~ type as parameter.
// 4 =[ptr] // after moving ownership, you have no access to the pointer!! it's dangling!


// # copying addresses

ptr :: 4 std.heap.alloc_bytes=>
q :: ptr    // doint this will move ownership and invalidate ptr! you cannot have a copy of it!
q :: &ptr   // but you can have an alias of it

// ptr std.heap.free=> // ptr has been invalidated!
q std.heap.free=>   // ok

// q =[ptr]  // q is also invalidated! it is dangling!



// # storing address to the stack

ptr :: 4 std.heap.alloc_bytes=>

s :: ptr =[]   // waive ownership and put it on stack. now 's' owns addr.

[s] std.heap.free=>    // now free this.



// # dealing with branches

ptr :: 4 std.heap.alloc_bytes=>
// if n is 1 -> ret    // oops! not moved for this branch! compile error!
// if n is 1 ->some_label    // it also needs to be moved before branching.
if n is 1 -> ptr std.heap.free=> ret

ptr std.heap.free=>


// # deferring

ptr :: 4 std.heap.alloc_bytes=>
~ ptr std.heap.free=> // note that ~ comes from destructor in c++
// ~ b is 0 foo=> you cannot have branches inside defer statement.
// also defer happens revesed order of declaration

// # deferring with branches
// if you are from go, it's very different how it works. it just pastes that statment before all branches that leaves current scope.

foo: (i32 d)
    ptr~ :: 4 std.heap.alloc_bytes=>
    ~ ptr~> std.heap.free=>


    d is 3 foo-> // pasted here, before branching
    d is 5 -> bar=> // NOT pasted before function call(the => part). also NOT pasted to conditional branch part (the -> part). these two does not leave this function!
    d is 7 -> ret // not pasted here, but sends control flow to the last line of the foo

    loop:   // note that we brought expanded version of @loop macro for clarity
        d(++) is 100 <- // NOT pasted
        loop->  // NOT pasted

    loop2:
        q :: 4 std.heap.alloc_bytes=>
        ~ q std.heap.free=> // for this defer statement...
        d(++) is 200 <- // gets pasted here (defer freeing q)
        loop->  // gets pasted here (defer freeing q)

    // pasted here, end of the fn



// # iterators
// pointer cannot be incremented by default (use type iter to make it incrementable)

Ptr :: 4 bytes std.heap.alloc=>
// Ptr(++) // you cannot change where Ptr points!
Iter :: Ptr as iter // iter type is not a owned address.
Iter(++)    // now you can change where it points!
[Iter(++)]  // *++Iter equivalant
[Iter](++)  // *Iter++ equivalant
[Iter(+ 4 bytes)]   // move by 4 bytes
// this iter type does not bound check by default. this is for c-style programming
// use std.array or other container types and use their iterators.



// # mutable pointers (*)
// pointer is not mutable by default, you need to put * when you mutate the value it is pointig to.
// use addr *Ptr to make it mutable
Ptr ::
    1234 std.heap.alloc.i32=>
~ . free=>
// 432 =[Ptr]   // compile error! changes the content without * notation
432 =[*Ptr] // ok, changes content to 432

set_to_zero: (addr i32 * =>)    // routine should denote that is mutates it
    0 =[*]

*Ptr set_to_zero=>  // also denote when passing to it
// you can just search *Ptr to see where it gets mutated!



// # shared pointers
// to make it copyable, or make it's ownership shared, use shared type.
// it is a atomically reference counted, which is equivalant to shared_ptr in c++ or Arc in rust

#alias std.shared  // just to make the name short
Shared_Resource :: 1234 shared.new=>
~. delete=> // still need to specify lifetime
// Copied :: Shared_Resource // you cannot just copy it!
Copied :: Shared_Resource shared.clone=>
~. delete=>
Weak :: Shared_Resource shared.weak=>
~. delete=>



// # working with libc malloc
// pointer is not nullable by default (addr? Ptr to make nullable)

ptr :: 4 libc.malloc=> // now you have to check for null, and free it later
// ~ ptr std.heap.free=> // make sure that you use proper function for freeing
~ ptr libc.free=>
// note that std.heap.alloc_bytes panics if it fails.
ptr isnt pointing -> panic // you have to check for null first. it's illegal to work with unchecked pointers


// # leaking

ptr :: 4 std.heap.alloc_bytes=>
ptr leak  // leak it if you need to.
// note that ptr is still invalidated after this.


// # noinit heap
ptr :: 4 std.heap.alloc_bytes.noinit=>
// the memory is wiped with 0 by default. if you want it to be not garenteed to wiped out, use this function

