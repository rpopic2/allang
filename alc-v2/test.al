#declare add: (Lhs i32, Rhs i32 => Sum i32)

[I] :: 3 =[]
J :: I
add [J], 3 =>


0

ret 0

add: (Lhs i32, Rhs i32 => Sum i32)
    ret Lhs + Rhs
