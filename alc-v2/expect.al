#declare printf: (Format addr u8 => Num_Printed i32)
#declare _Exit: (Status i32)

I :: 1
expect I is 1 =>

ret 0

expect: (Expect u8 =>)
    Expect isnt 0 -> printf "test failed\n" => _Exit 1 =>

expect_eq: (Lhs u8, Rhs u8 =>)
    Lhs isnt Rhs -> printf "test failed\n" => _Exit 1 =>

