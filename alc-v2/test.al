#declare foo: (X i32, Y i32 => A i32)
#declare printf: (Format addr u8 => Num_Printed i32)
#declare get_hi: (=> Hi addr u8)
#declare get_hi_redirected: (=> Hi addr u8)

printf "Hello World\n" =>

redirect:
Redirect :: get_hi_redirected =>
printf Redirect =>

V ::
    3 + 2 =
[Q] :: V =[]

[I] ::
    4 + 9
    2 + 1 =[]
[J] ::
    [^I] =[]
    2 + 9 =[]

offset:
[J, 2]
3 =[J, 1]

K ::
    2 + 33
    [P] ::
        5 =[]
    3 + 2 =

K is 3 ->
    P :: ^K
    K :: 0
    ret false

7 =[J]
1 + 3 =[J]

K is 2 ->
    ret true

I, 3, [I] [J]
Ko ::
    "hi" =

Ko, "ok"
foo 2, 2 =>
foo K, [J] =>

Hi :: get_hi =>
[Hi_Stack] :: get_hi => =[]

ret 0

foo: (X i32, Y i32 => A i32)
    ret X + Y

get_hi: (=> Hi addr u8)
    ret "HI\n"

store_char: (=>)
    Hi :: get_hi =>
    [C] :: [Hi] =[]

fn_assign: (=> R i32)
    Num_Printed ::
        printf "hello" => =
    Test :: 3
    ret Num_Printed

stack_var: (=> R i32)
    [X] :: 3 =[]
    ret [X]

get_hi_redirected: (=> Hi addr u8)
    ret get_hi =>


reg_off: (=>)
    O :: 2
    [X] ::
        3 + 2, 4 =[]
    [X, 2]
    Y :: X
    [Y, 1]
    [Y, O]
    3 =[Y]

copy: (=>)
    [X] :: 3 =[]
    [Y] :: [X] =[]
    ret

stack_store: (=>)
    [X] :: 3 =[]

short_lit: (=>)
    K ::
        u64{3 + 4} =
        2 =
    [S] ::
        u64{3 + 4} =[]
        ^K =[]

    2 =K
    u64{3 + 4} =K
    K =K
    // point{A, 4} =K
    // point{3, 4} =K
    2 =[S]
    u64{3 + 4} =[S]
    K =[S]
    ret

ret_int: (=> I i32)
    ret 1

assign_vals: (=>)
    I :: 1
    3 =I

    I2 :: I
    I =I2

    J ::
        2 =
        4 =This
        5 =^J

    J2 ::
        ^J =
        ^J =This

    ret

store_vals: (=>)
    [I] :: 1 =[]
    2 =[I]

    [I2] :: [I] =[]
    [I] =[I2]

    [PI] :: I =[]

    [J] ::
        3 =[]
        4 =[^J]
        5 =[This]

    [J2] ::
        [^J] =[]

fund_types: (=>)
    I8 :: i8{4 + 2}
    I16 :: i16{3}
    I32 :: i32{1}
    I64 :: i64{2}

    [I8_Stack] :: i8{5} =[]
    [I16_Stack] :: i16{6} =[]
    [I32_Stack] :: i32{7} =[]
    [I64_Stack] :: i64{8} =[]

    I8 =[I8_Stack]
    I16 =[I16_Stack]
    I32 =[I32_Stack]
    I64 =[I64_Stack]

    [I8_Stack] =I8
    [I16_Stack] =I16
    [I32_Stack] =I32
    [I64_Stack] =I64

    comptime_int:
    5 =I8

casting_i8: (=>)
    I8 :: i8{-3}
    I16 :: i16{I8}
    I32 :: i32{I8}
    I64 :: i64{I8}

    U8 :: u8{I8}
    U16 :: u16{I8}
    U32 :: u32{I8}
    U64 :: u64{I8}


casting_i16: (=>)
    Src :: i16{-3}

    I8 :: i8{Src}
    I32 :: i32{Src}
    I64 :: i64{Src}

    U8 :: u8{Src}
    U16 :: u16{Src}
    U32 :: u32{Src}
    U64 :: u64{Src}


casting_i32: (=>)
    Src :: i32{-3}

    I8 :: i8{Src}
    I16 :: i16{Src}
    I64 :: i64{Src}

    U8 :: u8{Src}
    U16 :: u16{Src}
    U32 :: u32{Src}
    U64 :: u64{Src}


casting_u8: (=>)
    Src :: u8{3}

    I8 :: i8{Src}
    I16 :: i16{Src}
    I32 :: i32{Src}
    I64 :: i64{Src}

    U16 :: u16{Src}
    U32 :: u32{Src}
    U64 :: u64{Src}


casting_u16: (=>)
    Src :: u16{3}

    I8 :: i8{Src}
    I16 :: i16{Src}
    I32 :: i32{Src}
    I64 :: i64{Src}

    U8 :: u8{Src}
    U32 :: u32{Src}
    U64 :: u64{Src}


casting_u32: (=>)
    Src :: u32{3}

    I8 :: i8{Src}
    I16 :: i16{Src}
    I32 :: i32{Src}
    I64 :: i64{Src}

    U8 :: u8{Src}
    U16 :: u16{Src}
    U64 :: u64{Src}


casting_u64: (=>)
    Src :: u64{3}

    I8 :: i8{Src}
    I16 :: i16{Src}
    I32 :: i32{Src}
    I64 :: i64{Src}

    U8 :: u8{Src}
    U16 :: u16{Src}
    U32 :: u32{Src}


casting_i64: (=>)
    Src :: i64{3}

    I8 :: i8{Src}
    I16 :: i16{Src}
    I32 :: i32{Src}

    U8 :: u8{Src}
    U16 :: u16{Src}
    U32 :: u32{Src}
    U64 :: u64{Src}

point: (=>)
    struct {
        X i8, Y i8, Z i8, W i8,
        A i8, B i8, C i8, D i8,
    }

point_test: (=>)
    make_1:
    P :: point{.X 3 .Y 5 .Z 1 .W 2 .A 1 .B 2 .C 3 .D 4}
    reg:
    X :: i8{3}
    Y :: i8{4}
    Z :: i8{5}
    W :: i8{6}
    make_2:
    O :: point{.X X .Y Y .Z Z .W W .A X .B Y .C 3 .D 4}
    zero:
    point{.B Y .. 0}

point16: (=>)
    struct {
        X i16, Y i16, Z i16, W i16,
    }

    X :: i16{1}
    Y :: i16{2}
    marker:
    P :: point16{.X X .Y Y .Z 3 .W 4}
    zero:
    point16{.. 0}
    point16{.Z X .. 0}

point32: (=>)
    struct {
        X i32, Y i32
    }

    X :: 1
    Y :: 2
    make:
    point32{.X X .Y Y}
    point32{.X X .Y 2}
    point32{.X 1 .Y Y}
    point32{.X 1 .Y 2}
    zero:
    point32{.Y 2 .. 0}
    point32{.X 2 .. 0}
    zero_reg:
    point32{.Y Y .. 0}
    point32{.X X .. 0}

struct64: (=>)
    struct {
        X i8, W i8, Y i16, Z i32
    }

    X :: i8{1}
    Y :: i16{2}
    Z :: i8{3}
    struct64{.X X .Y Y .W Z .. 0}
    zero:
    struct64{.Y Y .. 0}
    all_zero:
    struct64{.. 0}

struct_store: (=>)
    [S] :: point32{.X 1, .Y 2} =[]
    point32{.. 0} =[S]
