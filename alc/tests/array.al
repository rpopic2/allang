#declare printf: Format addr u8 => Num_Printed i32
#compile_all expect.al

[Arr] :: 5*i32{.0 0 .1 1 .2 2 .3 3 .4 4} =[]

Index :: 1

Index_U32 :: u16{2}

Arr * Index_U32 ! ret 3

Arr

[Arr.0]
[Arr.1]
[Arr.2]
[Arr.3]
[Arr.4]

Arr.0
Arr.1
Arr.2
Arr.3
Arr.4

[Arr * Index] ! ret 1

Arr * Index ! ret 2

// dynamic-index bounds via bare `*` (dyn_slice_access, array branch).
// The static array length is the compile-time bound here.

RA :: arr_dyn_inbounds =>
RA isnt 0 -> _Exit 50 =>

RB :: arr_dyn_last_valid =>
RB isnt 0 -> _Exit 51 =>

RC :: arr_dyn_at_len =>
RC isnt 99 -> _Exit 52 =>

RD :: arr_dyn_oob =>
RD isnt 99 -> _Exit 53 =>

// bare single-index `*` yields an element address (addr <elem>), loadable via [].

RE :: elem_addr_load =>
RE isnt 12 -> _Exit 54 =>

ret 0

arr_dyn_inbounds: => R i32
    [A] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: usize{2}
    A * Index ! ret 99
    ret 0

arr_dyn_last_valid: => R i32
    [A] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: usize{4}
    A * Index ! ret 99
    ret 0

arr_dyn_at_len: => R i32
    [A] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: usize{5}
    A * Index ! ret 99
    ret 0

arr_dyn_oob: => R i32
    [A] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: usize{10}
    A * Index ! ret 99
    ret 0

elem_addr_load: => R i32
    [A] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: usize{2}
    EA :: A * Index ! ret 99
    V :: [EA]
    ret V

