#compile_all expect.al
#declare _Exit: Status i32

[Arr] :: 5*i32{.. 0} =[]

Arr..

Arr.1..

Arr..1

Arr..5

Arr.1..1

Slice :: Arr..

I :: 1
J :: 2

// dynamic

dynamic =>

get_len Slice =>

slice_regression =>

slice_result_types =>

ret 0

dynamic: =>
    [Arr] :: 5*i32{.. 0} =[]
    I :: 1
    J :: 2

    S1 :: Arr * I.. ! ret

    S2 :: Arr * ..I ! ret

    Arr * I..J ! ret

get_at_three: S slice i32 => i32
    I :: [S.3] ! ret 1
    ret 0

get_len: S slice i32 => usize
    Size :: S.Length
    ret Size

// regression: a slice's runtime length survives being passed to a function,
// and dynamic-index bounds checks use that length (not the declarator amount).

slice_regression: =>
    [Buf] :: 5*i32{.. 0} =[]
    Slice :: Buf..
    Five :: usize{5}

    Slice.Length isnt Five -> _Exit 61 =>

    Len :: get_len Slice =>
    Len isnt Five -> _Exit 60 =>

    RA :: slice_dyn_inbounds Slice =>
    RA isnt 0 -> _Exit 62 =>

    RB :: slice_dyn_oob Slice =>
    RB isnt 99 -> _Exit 63 =>

slice_dyn_inbounds: S slice i32 => i32
    Begin :: usize{1}
    S * Begin.. ! ret 99
    ret 0

slice_dyn_oob: S slice i32 => i32
    Begin :: usize{10}
    S * Begin.. ! ret 99
    ret 0

// regression: a bare single index yields an element address (addr <elem>) that
// is loadable, while a range yields a slice <elem> whose Length is computed.

slice_result_types: =>
    [Buf] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Slice :: Buf..

    V :: slice_elem_addr Slice =>
    V isnt 12 -> _Exit 64 =>

    Four :: usize{4}
    L :: slice_subslice_len Slice =>
    L isnt Four -> _Exit 65 =>

slice_elem_addr: S slice i32 => i32
    Index :: usize{2}
    EA :: S * Index ! ret 99
    V :: [EA]
    ret V

slice_subslice_len: S slice i32 => usize
    Begin :: usize{1}
    Sub :: S * Begin.. ! ret 0
    ret Sub.Length
