#declare printf: (Format addr u8 => Num_Printed i32)

expect_eq 1, 2 =>
expect_eq 1, 1 =>

ret 0

expect_eq: (Lhs u8, Rhs u8 =>)
    Lhs isnt Rhs -> printf "test failed\n" =>

