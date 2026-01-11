#declare foo: (X i32, Y i32 => A i32)
#declare printf: (Format => Num_Printed)

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
    = 3 + 2

K is 3 ->
    P :: = ^K
    K :: = 0
    ret false

7 =[J]

K is 2 ->
    ret true

I, 3, [I], [J]
"hi", "ok"

foo 1, 2 =>

ret false

foo: (X i32, Y i32 => A i32)
    ret X + Y

