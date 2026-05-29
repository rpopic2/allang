// # constant array literals

| 0, 1, 2 | // a slice of readonly memory(embedded like the code inside binary(in TEXT segment)).
// (slice is a struct that has address and length)
i32 | 0, 1, 2 | // specify type of it
4 i32 | 1 | // specify length. note that this is not filled with 1. other members defaults to 0
4 i32 | 0: 1, 3: 5 | // put value to specific indicies
100 i32 | 1: 1 .. 50: 5 .. | // fill in range. keep in mind this is literal for readonly memory, generated in compile-time.

100 i32 | 1: { 1, 2, 3 } .. | // rest of them filled with 1, 2, 3. if you start from 0, it's compile error as 3 does not wrap in 100.

"hello" // strings are essentially same as the example above. it's a slice of readonly memory.
| 'h', 'e', 'l', 'l', 'o' | // basically shorthand for this.
c8 "hello" // utf-8 string
10 c8 "hellow" // string with length 10, remaining bytes zero filled.
100 c8 | 1: "lol" .. | // string filled with lols..
"hello world"0 // null terminated str

<< EOF
this is a heredoc string literal.
    "put whatever you want"\
eof can be changed to whatever you want
EOF

lit :: "literals"
i :: 23
`format string `lit` may be `i` useful`
// shorthand for
"format string %s may be %d useful", literal, i

// # creating static aggregates (probably not what you want unless you're optimising)
__data._static_array: DATA { 0, 1, 2 }   // now you can access this by __data.static_array from everywhere.
__data._some_array // it

// # creating external variables


// # load literal slices to register
a: | 1, 2, 3 | // array defined in TEXT segment, giving it a explicit symbol name.
arr :: a    // get slice of it
arr :: | 1, 2 | // get slice of it, alto defined in TEXT segment, anonymous.
arr :: | 1, 2 |.addr // get only address of it
arr :: | 1, 2 |.len // get only length of it
str :: "hello!" // get str of it. str is basically same as slice!
len :: "hello world".size // get only length of it. note that this is number of bytes of the string. this is why slice of string has ditinct type str.

a :: i32 [1, 2, 3, 4 ..] // warning: too big for single register!

// # load onto stack
// more on types.al

[parr] :: | 1, 2 | =[] // load slice of the array and stor on the stack. parr is the slice not the data!

[stack] :: [1, 2, 3, 100 ..] =[]  // this loads the array and store it to the stack.

[arr] :: [| 1, 2 |] =[] // loads the array from the readonly memory, storing it on stack.

heap_arr~ :: @(4 * #sizeof i32) std.heap.alloc_bytes=>
{ 1, 3, 5 } =[heap_arr] // loads array from the readonly memory, storing it on heap
heap_arr~ :: [ 1, 2, 3 ] @std.heap.alloc // or take shortcut

// declare one on stack.. technically not an array.

stack 10 i32 arr :: uninit // now it contains garbage. it's got wierd syntax because it is not recommended to do it. though it's unsafe, but you can still do it.

// just use zero array to zero it out. they are special type of array that does not actually live in readonly memory.
arr :: 10 i32 [] =[]

// [arr, i32 3] // stack arrays needs to be always bound checked. you cannot use this syntax unless this feature is turned off
arr[3] @at // use bound checked access.
arr.3 // or use the one bound-checked statically

// array access

parr :: | 1, 2, 3, 4 |
parr.0, parr.1 // address of first and second elem, bound checked statically.

// arr.i // you cannot do this. only numbers allowed, which is only for static use.
[parr.0] // get first element

i :: 0..parr.len @range //note that arr.len is known at comptime
    [arr, i32 i] // this syntax is not bound checked.


// not recommended way, might even not allow this later
{arr} :: { 1, 2, 3, 4 }
arr.0, arr.1 // data of first and second elem, bound checked statically.

// arr.i // you cannot do this. only numbers allowed, which is only for static use.
arr.0 // get first element

// i :: 0..arr.len @range // you can get len of it,
  //  [arr, i32 i] // but you cannot loop over it.

// i++, ++i-like behaviour

marr :: 10 std.heap.alloc_bytes
[marr(+ 1 addr)] // moves the ptr, this is like ++i
[marr](+ 1 addr) // like i++
[marr(+ 5 addr)] // add 5 and move marr
[maar](+ 3 addr) // load from marr and add 3 bytes
[maar](+ 1 auto)

[(+ 1 auto)maar]
[maar](+ 1 auto)

[(++auto)maar]

i(++)

i :: 0
@loop
    i(+ 1)
    arr[i] @at // use bound checked one
    is Error <- // stop if end of array
    print=>


// # structs

struct hi {
    u32 i, b8 b, // u8, u16 padding is here.
    u64 j
}
// declares a struct. pack it well as the memory layout is as it is declared

hi: // same stuff, but put into a label
    struct {
        u32 i, b8 b,
        u64 j
    }

// # struct literals. same as arrays!

hi | 8, true, 10 | // same, but it is not a slice, just data in readonly memory
hi | i = 10 , j = 8 | // rest of them zeroed out

| i64 header: MH_MAGIC_64, i32 cputype: CPU_TYPE_ARM64, i32 cpu_subtype: CPU_SUBTYPE_LITTLE_ENDIAN |

arr :: { 1, 2 } // load onto a register..but actually optimized always to moving it onto it. more on it later.
// arr :: 5 { 1, 2 } // compile err: exceeds 8 bytes!
{arr} :: 5 { 1, 2 } // ok, stored in multiple named registers.. but not recommended
arr :: { 1, 2 } // load onto a register
x, y :: { 1, 2 } // you can also destructure it

// what if...

struct x {
    u64 hi, a, b, c, d, e, f, g
}
[sobj] :: x {} =[] // requires 2 type q stp
{ r } :: x // well, it might in be multiple regs...





// loading onto registers

s :: hi | 8, true | // movs address to it to reg
s :: hi.size // do this to get size
{s} :: hi { 8, false, 9 } // loads data onto multiple registers
i, b, j :: { 8, false, 9 } // destructures it

// loading onto stack

[s] :: hi { 10, true } =[] // stores data
[ps] :: hi | 10, true | =[] // stores slice


heap_arr~ :: #sizeof hi std.heap.alloc_bytes=>
{ 1, true, 5 } =[heap_arr] // loads array from the readonly memory, storing it on heap
heap_arr~ :: { 1, 2, 3 } @std.heap.alloc // or take shortcut

// declaring on the stack

stack hi [h] :: uninit // same stuff. contains garbage, not recommended.

[h] :: hi {} =[] // also use zero struct to init it.

// struct member access
h.i, h.b, h.j

// these are also allowed.
h.0, h.1, h.2
h.x, h.y, h.z // and h.w
// h.r, h.g, h.b // and h.a.. but this does not work because it has member named b

// # tuples and destructions

foo: (i32, i32 =>i32, i32)
    +, -

a, b :: 1, 2 foo=>
    // { 1, 2 } foo=> // type mismatch!! second arg also missing
// so far so good! return type need not be tuple.


struct point {
    i32, i32
}

bar: (point =>point)
    { x, y }    // destructures it
    { +, - } // reconstructs it

// 1, 2 bar=> // too many args
p :: { 1, 2 } bar=> // type inferred
p.x, p.y // access it
// p foo=> // type mismatch!
p { a, b } foo=> // ok
s :: 2 i32 { 1, 2 }
// s bar=> // type mismatch! type was { i32, i32 }
point { s.x, s.y } bar=> // ok, but wierd

// this @macro expansion section is wip.
struct pointwo 2 i32

baz: (@point =>@point)
    +, -
// now identical signature with foo, but accepts anything that can be destructured like it!

p :: point { 1, 2 }
pt :: pointwo { 3, 4 }
1, 2 baz =>
{ 1, 2 } baz=>
p baz=>
pt baz=>

r :: p baz=> // ok, constructed to type of point
pt :: p baz=> // ok, pt is type of pointwo, constructed
x, y :: p baz=> // ok.

// # union

@union uni {
    i32 a,
    u64 b,
    c8 c,
}

// literal for union in readonly memory is same with struct

uni { i32 3 }
uni { a: 3 }

u :: uni { i32 3 }
u.a // contains 3 as i32
u :: uni { u64 3 }
u.b // contains 3 as u64
u.c // b is selected! compile err!

// foo: (uni u =>uni)   // you cannot pass in union across fns!
//     u.a is 5 -> uni { b: 87 } // because you don't know which one is selected!!

// union by itself seems not useful, but...

// # enum union
// enum union is a union, but holding more byte(s) for an enum

@union u {
    void Error
    i32 Ok
}
#enum e i32 {
    Error, Ok
}
@struct enum_union {
    e enum,
    d data,
}

eu :: enum_union { e: #Ok, d: { i32 5 } }
eu :: enum_union { e: #Ok, d: { Ok: 5 } }
eu :: enum_union { e: #Error, d: { } }
eu :: enum_union { #Error }
eu :: enum_union { #Ok, 3 }

foo: (i32 i =>enum_union)
    i < 42 ->
        { d: i, e: #Ok } ret
    { e: #Error } ret

eu :: 42 foo=>
eu.err is #Error -> "error!" print=>,
err.err is #Ok -> err.value print=>

err, value :: 1 foo=>
err is #Error -> "error!" print=>,
err is #Ok -> value print=>

eu :: 42 foo=>
eu is { #Error } -> "error!" print=>,
eu is { #Ok } -> eu.v print=>   // do pattern matching
eu is { #Ok, v } -> v print=>   // do pattern matching

// declare it in shorthand
@enum union Result {
    void Error
    i32 Ok,
}

// for null pointer, use 'addr?' type. '?' denotes that its null value is zero. it's a compile-time error if you do not check aginst null.

ptr? :: 4 libc.malloc=>
ptr is null -> panic    // there must be 'is null' or 'isnt null' somewhere.

4 =[ptr]


// # built in types of aggregates

slice T:
    struct {
        addr T addr,
        usize len,
    }

fat T:
    struct {
        addr T addr,
        usize size,
    }

str:
    struct {
        addr c8 addr,
        usize size,
    }

double T:
    struct {
        addr T begin,
        addr T end]
    }

@generic option.(type T):
    enum union {
        void Error,
        T Ok
    }

// # sizeof, countof

a: | 1, 2, 3, 4, 5 |
#sizeof a // 8 becaus a is a slice
#countof a // 2 because a is a slice
a.len // 5
a.len * #sizeof i32 // 20
a @slice.sizeof // 20


// len is count of elements in slice.
// size is size in bytes
test "len and size":
    slice i32 |1, 2, 3| $
    $.len is 3 @expect
    size :: len * #sizeof i32 is 6
    size is 12 @expect

    #sizeof (slice i32) is 8 @expect
    #countof 
