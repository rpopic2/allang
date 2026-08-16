#compile_all expect.al
#declare printf: Format addr u8 => Num_Printed i32

// Exercises the expect() helper from expect.al as an assertion mechanism.
// expect() takes the boolean result of a comparison (1 = pass, 0 = fail);
// a false condition calls _Exit 1. Every assertion here is true, so the
// program must run to `ret 0`. This drives the comparison-as-value path
// (compare_branch with a PARAM destination / emit_cond_set), which the
// branch-based tests in index_test.al do not cover.
//
// Assertions are split across small functions so each gets a fresh register
// budget.

const_cmp =>
runtime_cmp =>
arith_cmp =>
signed_cmp =>
width_cmp =>

printf "all expect tests passed\n" =>
ret 0

// constant-folded conditions (lhs and rhs both compile-time values)
const_cmp: =>
    C :: 5
    expect C is 5 =>
    expect C isnt 4 =>
    expect C < 10 =>
    expect C > 1 =>
    expect C <= 5 =>
    expect C >= 5 =>

// runtime values loaded from memory (emit_cond_set path)
runtime_cmp: =>
    [Mem] :: 5 =[]
    V :: [Mem]
    expect V is 5 =>
    expect V isnt 4 =>
    expect V < 10 =>
    expect V > 1 =>
    expect V <= 5 =>
    expect V >= 5 =>

// expect over arithmetic results
arith_cmp: =>
    [Mem] :: 5 =[]
    V :: [Mem]
    Sum :: V + 3
    expect Sum is 8 =>
    Diff :: V - 2
    expect Diff is 3 =>

// signedness: high-bit-set unsigned compares as a large positive,
// signed negative compares as below zero
signed_cmp: =>
    U :: u32{0x80000000}
    expect U > 1 =>
    expect U >= 1 =>
    N :: i32{-1}
    expect N < 0 =>
    expect N <= 0 =>

// other integer widths
width_cmp: =>
    W8 :: u8{200}
    expect W8 > 100 =>
    W16 :: i16{7}
    expect W16 is 7 =>
    W64 :: i64{-5}
    expect W64 < 0 =>
