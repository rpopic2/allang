#declare printf: Format addr u8 => Num_Printed i32
#declare _Exit: Status i32

// Exercises the unified `=[]` stack-store path (binary_op_store / do_store).
// The empty-target store used to route through a separate stmt_stack_store
// path; these checks verify each source form lands the right value:
// untyped-immediate (emit_str_imm + comptime rsize), typed-immediate,
// binary-op-led, and load-led stores. The chained case verifies a single
// source value lands in every target of `v =[A] =[B]`. The register-source
// case stores through a pointer parameter, whose base register must keep its
// full width in the memory operand.

R0 :: imm_untyped =>
R0 isnt 7 -> _Exit 110 =>

R1 :: imm_typed =>
R1 isnt 42 -> _Exit 111 =>

R2 :: binary_led =>
R2 isnt 5 -> _Exit 112 =>

R3 :: load_led =>
R3 isnt 7 -> _Exit 113 =>

[S] :: pair{.. 0} =[]
R4 :: chained S =>
R4 isnt 7 -> _Exit 114 =>

[T] :: pair{.. 0} =[]
R5 :: reg_through_addr T =>
R5 isnt 9 -> _Exit 115 =>

printf "all store tests passed\n" =>
ret 0

imm_untyped: => R i32
    [X] :: 7 =[]
    ret [X]

imm_typed: => R u32
    [X] :: u32{42} =[]
    ret [X]

binary_led: => R i32
    [X] :: 3 + 2 =[]
    ret [X]

load_led: => R i32
    [X] :: 7 =[]
    [Y] :: [X] =[]
    ret [Y]

pair:
    struct { A i32, B i32 }

chained: P addr pair => R i32
    7 =[P.A] =[P.B]
    ret [P.B]

reg_through_addr: P addr pair => R i32
    V :: i32{9}
    V =[P.A]
    ret [P.A]
