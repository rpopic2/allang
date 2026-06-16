#declare printf: Format addr u8 => Num_Printed i32
#declare _Exit: Status i32

// The bounds-check operator `!` returns from the function when the index is
// out of bounds. Every existing `!` test uses an in-bounds index, so the
// failure branch is never actually taken. These exercise both outcomes.

R0 :: load_inbounds =>
R0 isnt 12 -> _Exit 40 =>

R1 :: load_oob =>
R1 isnt 99 -> _Exit 41 =>

R2 :: load_at_len =>
R2 isnt 99 -> _Exit 42 =>

R3 :: load_last_valid =>
R3 isnt 14 -> _Exit 43 =>

R4 :: slice_inbounds =>
R4 isnt 0 -> _Exit 44 =>

R5 :: slice_oob =>
R5 isnt 99 -> _Exit 45 =>

printf "all bounds tests passed\n" =>
ret 0

load_inbounds: => R i32
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: usize{2}
    [Arr * Index] ! ret 99
    V :: [Arr * Index unchecked]
    ret V

load_oob: => R i32
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: usize{10}
    [Arr * Index] ! ret 99
    V :: [Arr * Index unchecked]
    ret V

load_at_len: => R i32
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: usize{5}
    [Arr * Index] ! ret 99
    V :: [Arr * Index unchecked]
    ret V

load_last_valid: => R i32
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: usize{4}
    [Arr * Index] ! ret 99
    V :: [Arr * Index unchecked]
    ret V

slice_inbounds: => R i32
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Begin :: usize{1}
    Arr * Begin.. ! ret 99
    ret 0

slice_oob: => R i32
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Begin :: usize{10}
    Arr * Begin.. ! ret 99
    ret 0
