#declare foo: (X i32, Y i32 => A i32)
#declare printf: (Format u64 => Num_Printed i32)

printf "Hello World\n" =>

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

K is 2 ->
    ret true

I, 3, [I], [J]
"hi", "ok"

foo 2, 2 =>
foo K, [J] =>

ret false

foo: (X i32, Y i32 => A i32)
    ret X + Y

fn_assign: (=> R)
    Num_Printed ::
        printf "hello" => =
    Test :: = 3
    ret Num_Printed

stack_var: (=> R)
    [X] :: 3 =[]
    ret 0

reg_off: (=>)
    O :: = 2
    [X] ::
        ^X
        4 =[^X]
        =[]
    [X, 2]
    Y :: = X
    [Y, 1]
    [Y, O]
    3 =[Y]

copy: (=>)
    [X] :: 3 =[]
    [Y] :: [X] =[]
    ret

cool:
    struct {
        X i32, Y i32
    }

