#declare foo: (X i32, Y i32 => A i32)
#declare printf: (Format addr u8 => Num_Printed i32)
#declare get_hi: (=> Hi addr u8)
#declare get_hi_redirected: (=> Hi addr u8)

printf "Hellow World\n" =>

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

    comptime_int:
    5 =I8

casting: (=>)
    I8 :: i8{-3}
    I16 :: i64{I8}


