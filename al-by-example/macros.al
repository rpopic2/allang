// inline macros

@inline loop:
    :
        ~ ->.
        continue:

@loop
    "hi" printf=>

// expands to 

:
    ~ ->.
    continue:
    "hi" printf=>

test: @loop
// expands to
test :  // now you have named loop
    // ...

@loop; ~ defer_sth=>
// expands to
:   ; ~defer_sth
    // ...

// https://zig.guide/language-basics/comptime

fibonacci: (u16 n =>u16)
    is 0 or, 1 -> n ret
    a :: n - 1 fibonacci=>
    b :: n - 2 fibonacci=>
    a + b ret

test "comptime blocks"
    x: #compime 10 fibonacci=>
    y:
        #comptime 10 fibonacci=>

test "branching on types"
    a :: 5
    t :: a < 10 ? f32 : i32
    t a :: 5

Matrix: (type T, comptime_int width, comptime_int height =>type)
    height width T

Matrix(f32, 4, 4) is 4 4 f32

allSmallInts: (type T, T a, T b =>T)
    T @typeinfo
    ret
        is ComptimeInt -> a + b,
        is (Int, info) ->
            info.bits <= 16 -> a + b : "ints too large" @compileError
        : "only ints accepted" @compileError

test "typeinfo switch"
    x: u16, 20, 30 addSmallInts=>
    @typeof is u16
    x is 50

GetBiggerInt(type T =>type)
    $ :: T @typeInfo
    { Int: { bits: $.Int.bits + 1, signedness: $.Int.signedness } @Type

expect GetBiggerInt(u8) is u9  // this is sick...
expect GetBiggerInt(u31) is i32

Vec: (comptime_int count, type T =>type)
    @struct {
        count T data,
    }
    Self :: @this
    abs: (Self self =>Self)
        tmp :: Self { data: undefined }
        self.data, T elem, i32 i @foreach.index
            tmp.data[i] @at =
                elem < 0 ? -elem : elem
        tmp ret

    init: (count T data =>Self)
        Self { data: data }

test "generic vector"
    x :: 3, f32 Vec=> f32 { 10, -10, 5 } init=>
    y :: x abs=>

// https://odin-lang.org/docs/overview/#when-statement

#if ODIN_ARCH
    is .i386 -> "32 bit"
    is .amd64 -> "64 bit"
    : "unsupported arch"
print=>

// compiles to
"64 bit"
print=>


