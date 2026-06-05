#declare printf: Format addr u8 => Num_Printed i32
#declare _Exit: Status i32

[Arr] :: 5*i32{.0 0 .1 1 .2 2 .3 3 .4 4} =[]

expect [Arr.0] is 0 =>
expect [Arr.1] isnt 0 =>
expect [Arr.3] is 3 =>

Sum :: [Arr.2] + 1
expect Sum is 3 =>

I :: test_struct{.Foo 1}
I.Foo isnt 1 ->
    printf "test failed\n" => _Exit 1 =>

[Arr.4] is 4 ->
    ret 0
ret 1

expect: Expect i32 =>
    Expect is 0 ->
        printf "test failed\n" => _Exit 1 =>

expect_eq: Lhs u8, Rhs u8 =>
    Lhs isnt Rhs -> printf "test failed\n" => _Exit 1 =>

test_struct:
    struct {
        Foo u8
    }
