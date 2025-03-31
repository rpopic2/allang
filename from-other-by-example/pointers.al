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

// https://zig.guide/language-basics/pointers

increment: (addr u8 num)
    [num] + 1 =[]

test "pointers"
    |stack u8 x| :: 1 []=
    increment=>

    expect x is 2

test "naughty pointer"
    |u16 x| :: 5
    x-= 5
    addr u8 y :: x as addr u8   // warning: creating ptr from unowned ptr
    [y] = y // null ptr is allowed in allang

    stack u8 x :: 1 =[]
    |y| :: x    // now the type is |addr| u8, pointer to const u8
    // [y] + 1 =[] err! cannot store to const ptr

// we don't have many item ptr
doubleAllManypointer: (addr u8 buffer, usiz byte_count)
    usiz i :: 0
    i < byte_count, i+= 1 @zig.while
        [buffer, i u8] * 2 =[]

test "many-item pointers"
    stack |100 u8 buffer| :: 100 { 1 } =[]
    addr |100 u8 buffer_ptr| :: buffer

    manyaddr u8 buffer_many_ptr :: buffer_ptr
    buffer_many_ptr, #len buffer doubleAllManypointer=>

    buffer, byte @foreach
        expect byte is 2

    addr u8 first_elem_ptr :: buffer_many_ptr + 0
    addr u8 first_elem_ptr_2 :: buffer_many_ptr

// http://odin-lang.org/docs/overview/#pointers

p :: addr i32 not pointing
i :: 123 =[]
p :: i

[p] print=>
1337 =[p]

// pointer arithmetic exist.

