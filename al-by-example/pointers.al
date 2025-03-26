// https://gobyexample.com/pointers
// pointers are called just address in al.
// the type is called addr for short.
// keep in mind it is just a mere 64-bit unsigened integer actually

zeroval: (i32 ival)
    0 =ival

zeroptr: (addr i32 iptr)
    0 =[iptr]

i :: 1

i zeroval=>
// i zeroptr=> // is illeagal because i is a register!

[i]= :: 1   // you have to allocate it on the stack before you can even THINK about taking address of it!

// i zeroval=> // this is illeagal because now i is an offset from stack!
[i] zeroval=>   // you have to dereference call it!
i zeroptr=>     // while i is technically a relative address, it is trivial to calculate it on runtime, and there is no need to distinguish between those two.

i :: #sizeof i32 std.heap.alloc=> // now i holds addr to some location on the heap
[i] zeroval=>
i zeroptr=>


