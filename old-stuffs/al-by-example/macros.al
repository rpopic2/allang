"hello world" print=>


// # inline

// please use inline sparingly. only inline small functions.
@inline at: (slice s, i32 index =>option i32) // type checked inline fns.
    index >= s.len
        -> { Error } ret
    { Ok, [s.addr, i32 index] }

s: |1, 2, 3|

s, 1 @at    // expands to...

1 >= s.len
    -> { Error }
{ Ok, [s.addr, i32 1] }

s.1 // just use this for static check, it's much more simple...
// invoke macros carefully! there might be quite a lot behind @ macro!

// macros support nice forms of providing arguments
@inline at: (slice s[i32 index] =>option i32) // subscript like macro
    // snip

s[1] @at


@inline at: (slice s..i32 index =>option i32) // range like macro
s..1 @at

// # generic

@inline list.(@type T):
    struct {
        addr T data,
        i32 len,
    }

@list.i32   // you have to expand this macro somewhere
// expands to
list.i32:
    struct {
        addr i32 data,
        i32 len,
    }

// just use it like any other types
stack list.i32 ls :: noinit

// or use shorthand

@generic list.(@type T):
    struct {
        addr T data
        i32 len
    }




// # merging or embedding structs

struct foo {
    i32 i, j
}

struct bar // merged as if deriving from it
    @foo,
    { i32 a, b }

    // expands to
    // struct bar
    //     foo { i32 i, j },
    //     bar { i32 a, b }

b :: bar { i: 1, j: 2, a: 3, b: 4 }
b :: bar { foo.i: 1, foo.j: 2, bar.a: 3, bar.b: 4 }

struct baz {    // embedded in the struct
    @foo f,
    i32 a, b
}
    // struct baz {
    //     { i32 i, j } f,
    //     i32 a, b
    // }

bz :: baz { f.i: 1, f.b: 2, a: 3, b: 4 }

// you don't have to expand macro yourself, and it will be only expanded once for each type.
// use @inline only if you need to specify where it gets expanded

debug_print: (@foo p)
    `has value of: `p.i`, y: `p.j print=>
    // expands to...
    // debug_print: ({f64 p.x, f64 p.y, f64 p.z})
    //     `has value of x: `p.x`, y: `p.y`, z: `p.z print=>

foo { 1, 2 } debug_print=>  // prints 1 and 2
bar { 5, 6, 3, 4 } debug_print=> // prints 5 and 6
baz { { 7, 8 }, 3, 4 } debug_print=> // prints 7 and 8
// if this reminds of you oop, look oop.al

// # value replacement macros
// thi macro do simple replcement for values

#sizeof i32 print=>
// expands to
4 print=>

enum hi {
    foo, bar, baz
}

#foo print=>    // 0
foo print=>     // foo

#define hello 0 // these defines are type checked, 0 defaults to i32, so hello is i32.
// i64 j :: hello  // error! HELLO is i32

#alias print std.print
print=>

#alias std.print    // same stuff, uses last word as aliased name
print=>

#self   // gets replaced to current scope's name.
// expands to main
