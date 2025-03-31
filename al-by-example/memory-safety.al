// first of all, al is manually allocated language.
// programmer is responsible of managing memory...
// but al can help programmers doing that!

// # owned address

ptr~ :: 4 std.heap.alloc_bytes=> // '~' denotes that it's a owning addr. you will have to free this addr, or move ownership to something else.

// ptr :: 4 std.heap.alloc_bytes=> // syntax error! alloc_bytes returns owned address. '~' is missing!

ptr~> std.heap.free=>   // ~> waives ownership. it moves ownership to free fn
// 4 =[ptr] // after moving ownership, you cannot acces the pointer!! it's dangling!

ptr~ : 4 std.heap.alloc_bytes=> // but you can assign new value to it!

// if you do not move pointer, it's compile error!



// # copying addresses

ptr~ :: 4 std.heap.alloc_bytes=>
q :: ptr    // you can have a copy of it

// q std.heap.free=> // compile error! q is not owned. it is also illegal to make it 'q~> std.heap.free'.
ptr :: std.heap.free=> // all copies of ptr are invalidated.

// q =[ptr]  // q is also invalidated! it is dangling!



// # storing address

ptr~ :: 4 std.heap.alloc_bytes=>
q :: ptr

[s~] :: ptr~> =[]   // waive ownership and put it on stack. now 's' owns addr.
// [s] :: ptr =[] // you cannot store owned pointers

addr~ i32 [s] :: noinit
ptr ~>[s]

[s~] std.heap.free=>    // now free this.



// # dealing with branches

ptr~ :: 4 std.heap.alloc_bytes=>
// if n is 1 -> ret    // oops! not moved for this branch! compile error!
// if n is 1 ->some_label    // it also needs to be moved before branching.
if n is 1 -> ptr~> std.heap.free=> ret

ptr~> std.heap.free=>


// # deferring

ptr~ :: 4 std.heap.alloc_bytes=>
~ ptr~> std.heap.free=> // note that ~ comes from destructor in c++
// ~ b is 0 foo=> you cannot have branches inside defer statement.
// also defer happens revesed order of declaration

// # deferring with branches
// if you are from go, it's very different how it works. it just pastes that statment before all branches that leaves current scope.

foo: (i32 d)
    ptr~ :: 4 std.heap.alloc_bytes=>
    ~ ptr~> std.heap.free=>


    d is 3 ->foo // pasted here, before branching
    d is 5 -> bar=> // NOT pasted before function call(the => part). also NOT pasted to conditional branch part (the -> part). these two does not leave this function!
    d is 7 -> ret // pasted here, just before ret statement.

    loop:   // note that we brought expanded version of @loop macro for clarity
        d(++) is 100 <- // NOT pasted
        ->loop  // NOT pasted

    loop2:
        q~ :: 4 std.heap.alloc_bytes=>
        ~ q~> std.heap.free=> // for this defer statement...
        d(++) is 200 <- // gets pasted here (defer freeing q)
        ->loop  // gets pasted here (defer freeing q)

    // pasted here, end of the fn



// # working with libc malloc

ptr~? :: 4 libc.malloc=> // now you have to check for null, and free it later
// note that std.heap.alloc_bytes panics if it fails.
ptr is null -> panic // you have to check for null first. it's illegal to work with unchecked pointers
~ ptr~> libc.free=>
// ~ ptr~> std.heap.free=> //it's illegal because ptr is type of addr~?


// # leaking

ptr~ :: 4 std.heap.alloc_bytes=>
ptr~> leak  // leak it if you need to.
// note that ptr is still invalidated after this.


// # noinit heap
ptr~ :: 4 std.heap.alloc_bytes.noinit=>
// the memory is wiped with 0 by default. if you want it to be not garenteed to wiped out, use this function


// we have two versions of linked list to demonstrate this.
// check out linked-list.al
