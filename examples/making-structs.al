goals:
- parses fast
- can be disassembled to
- don't make people confused

1. familiar. easier to parse
J = 3 + 5
J = 3 + I
K = I + J
K = [J]
K = fn =>
K = I
K = "HI"
K = 3

2. one rule for assignment. needs lookup
3 + 5 =J
3 + I =J
I + J =K
[J] =K
fn=> =K
I =K
"HI" =K
3 =K

2-1.
3 + 5 = J
3 + I = J
I + J = K
[J] = K
fn=> = K
I = K
"HI" = K
3 = K

3. looks simpler(maybe..?)
J(3 + 5)
J(3 + I)
K(I + J)
K([J])
fn=> K()
K(I)
K("HI")
K(3)

4. one rule for assignment
3 + 5 =[J]
3 + I =[J]
I + J =[K]
[J] =[K]
fn => =[K]
I =[K]
"HI" =[K]
3 =[K]

5. may be familiar
[J]= 3 + 5
[J]= 3 + I
[J]= [K]
fn => =[K]
[K]= I
[K]= "HI"
[K]= 3

make struct 

[K] :: my_struct{1, 2, C, K}
J :: short_struct(1, 2, C, K)
O :: view<1, 2, 3>

struct point {
    u64[](X, Y, Z, W, K, P, A, B)
}

R :: ppoint 3, 4 => =

ppoint: (K i32, J i32 => =Self addr point)
    [u64[]<2, 3>] =[Self.X..Y]
    (4, u64(K)) =[Self.Z..W]
    (u64(J), 0) =[Self.K..P]
    0 =[Self.A..B]
    ret

ppoint: (K i32, J i32 => =Self addr point)
    // macro way
    ppoint{2, 3, 4, u64(K), u64(J)} =[Self]

    // m2 way
    [u64[]<2, 3>] =[Self.X..Y]
    (4, u64(K)) =[Self.Z..W]
    (u64(J), 0) =[Self.K..P]
    0 =[Self.A..B]
    ret

    // naïve raptor lake way
    [u64[]<2, 3>] =[Self.X..Y]
    4 =[Self.Z]
    (u64(J), u64(K)) =[K..J]
    0 =[Self.P]
    0 =[Self.A..B]

    // a72 way
    const_pool: <2, 3>
    (4, u64(K)) =[Self.Z..W]
    $pool_page :: #intrinsic adrp const_pool
    (u64(J), 0) =[Self.K..P]
    [pool_page, #intrinsic lo12 const_pool] =[Self.X..Y]
    0 =[Self.A..B]

    // a720 way
    (2, 3) =$$XY
    (4, u64(K)) =[Self.Z..W]
    u64(J) =$$K
    0 =[Self.A..B]
    (K, 0) =[Self.K..P]
    XY =[Self.X..Y]

    // skylake way
    [u64[]<2, 3>] =$$XY, #intrinsic vmovd $$W, K
    4 =[Self.Z]
    Self =$ret
    0 =[Self.P]
    $XY =[Self.X..Y]
    #intrinsic vpinsrd $$KJ, $W, J, 1
    #intrinsic vpmovsxdq $KJ $KJ
    $KJ =[K..J]
    0 =[Self.A..B]


    // skylake way 2
    [u64[]<2, 3>] =$$XY
    4 =[Self.Z]
    Self =$ret
    0 =[Self.P]
    $XY =[Self.X..Y]
    (u64(J), u64(K)) =$$KJ
    $KJ =[K..J]
    0 =[Self.A..B]


    // raptor lake way
    [u64[]<2, 3>] =$$XY
    Self =$ret
    4 =[Self.Z]
    0 =[Self.P]
    $XY =[Self.X..Y]
    (u64(J), u64(K)) =$$KJ
    $KJ =[K..J]
    0 =[Self.A..B]

    arr[0..4] + arr[4..8]
    | arr[0] + arr[1] + arr[2] + arr[3] |
    arr[0..4] acc

    [arr, 0..4] + [arr, 4..8]
    u32|0, 1, 2, 3|
    Simd :: [arr, 0 + 1 + 2 + 3]
    Simd 



