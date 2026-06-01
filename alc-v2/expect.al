#declare printf: (Format addr u8 => Num_Printed i32)
#declare _Exit: (Status i32)

I :: test_struct{.Foo 1}
I.Foo is 1 ->
    ret 0
ret 1

expect: (Expect u8 =>)
    Expect is 0 ->
        printf "test failed\n" => _Exit 1 =>

expect_eq: (Lhs u8, Rhs u8 =>)
    Lhs isnt Rhs -> printf "test failed\n" => _Exit 1 =>

test_struct:
    struct {
        Foo u8
    }
