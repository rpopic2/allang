#declare add: (Lhs i32, Rhs i32 => Sum i32)

[I] :: 3 =[]
J :: "hi"
K :: [J]
add 2, 3 =>


0

ret 0

add: (Lhs i32, Rhs i32 => Sum i32)
    ret Lhs + Rhs

case1: (=>)
    1 + 2

case2: (=>)
    [I] :: 3 =[]

case3: (Lhs i32 => Sum i32)
    case1 =>
    ret Lhs

case4: (Lhs i32, Rhs i32 => Sum i32)
    case1 =>
    J :: 0
    [I] :: Lhs =[]
    ret Lhs + Rhs
