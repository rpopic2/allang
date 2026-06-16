#declare printf: Format addr u8 => Num_Printed i32
#declare _Exit: Status i32

// Arithmetic result verification. test.al exercises these forms but only
// indirectly verifies REG+VALUE / REG-VALUE; the VALUE-REG paths (which use
// a separate scratch-register sequence) and `shl` are never checked at
// runtime.

R0 :: add_val_reg =>
R0 isnt 12 -> _Exit 100 =>

R1 :: sub_reg_val =>
R1 isnt 6 -> _Exit 101 =>

R2 :: sub_val_reg =>
R2 isnt 7 -> _Exit 102 =>

R3 :: sub_reg_reg =>
R3 isnt 6 -> _Exit 103 =>

sub_neg =>

R5 :: shl_check =>
R5 isnt 48 -> _Exit 105 =>

R6 :: shl_zero =>
R6 isnt 3 -> _Exit 106 =>

printf "all arith tests passed\n" =>
ret 0

add_val_reg: => R i32
    X :: i32{5}
    R :: 7 + X
    ret R

sub_reg_val: => R i32
    X :: i32{10}
    R :: X - 4
    ret R

sub_val_reg: => R i32
    X :: i32{3}
    R :: 10 - X
    ret R

sub_reg_reg: => R i32
    X :: i32{10}
    Y :: i32{4}
    R :: X - Y
    ret R

// VALUE - REG yielding a negative result, then a signed comparison.
sub_neg: =>
    X :: i32{5}
    R :: 3 - X
    R < 0 ->
        ret
    _Exit 104 =>

shl_check: => R i32
    I :: i32{3}
    R :: I shl 4
    ret R

shl_zero: => R i32
    I :: i32{3}
    R :: I shl 0
    ret R
