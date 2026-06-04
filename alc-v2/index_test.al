#declare printf: Format addr u8 => Num_Printed i32
#declare _Exit: Status i32

elem_i32 =>
elem_i64 =>
elem_u32 =>
elem_u64 =>

dyn_i8 =>
dyn_i16 =>
dyn_i32 =>
dyn_i64 =>
dyn_u8 =>
dyn_u16 =>
dyn_u32 =>
dyn_u64 =>
dyn_usize =>

checked_i8 =>
checked_i16 =>
checked_i32 =>
checked_i64 =>
checked_u8 =>
checked_u16 =>
checked_u32 =>
checked_u64 =>
checked_usize =>

slice_i8 =>
slice_i16 =>
slice_i32 =>
slice_i64 =>
slice_u8 =>
slice_u16 =>
slice_u32 =>
slice_u64 =>
slice_usize =>

printf "all index tests passed\n" =>
ret 0

// --- static indexing of arrays of word-sized integer element types ---
// (narrow i8/i16/u8/u16 element arrays are skipped: their aggregate
//  initialization is broken by a separate pre-existing bug.)

elem_i32: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    A :: [Arr.0]
    A isnt 10 -> _Exit 12 =>
    B :: [Arr.2]
    B isnt 12 -> _Exit 12 =>
    C :: [Arr.4]
    C isnt 14 -> _Exit 12 =>

elem_i64: =>
    [Arr] :: 5*i64{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    A :: [Arr.0]
    A isnt 10 -> _Exit 13 =>
    B :: [Arr.2]
    B isnt 12 -> _Exit 13 =>
    C :: [Arr.4]
    C isnt 14 -> _Exit 13 =>

elem_u32: =>
    [Arr] :: 5*u32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    A :: [Arr.0]
    A isnt 10 -> _Exit 16 =>
    B :: [Arr.2]
    B isnt 12 -> _Exit 16 =>
    C :: [Arr.4]
    C isnt 14 -> _Exit 16 =>

elem_u64: =>
    [Arr] :: 5*u64{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    A :: [Arr.0]
    A isnt 10 -> _Exit 17 =>
    B :: [Arr.2]
    B isnt 12 -> _Exit 17 =>
    C :: [Arr.4]
    C isnt 14 -> _Exit 17 =>

// --- dynamic indexing of an i32 array with an index of every integer type ---

dyn_i8: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: i8{2}
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 20 =>

dyn_i16: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: i16{2}
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 21 =>

dyn_i32: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: i32{2}
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 22 =>

dyn_i64: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: i64{2}
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 23 =>

dyn_u8: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: u8{2}
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 24 =>

dyn_u16: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: u16{2}
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 25 =>

dyn_u32: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: u32{2}
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 26 =>

dyn_u64: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: u64{2}
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 27 =>

dyn_usize: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: usize{2}
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 28 =>

// --- bounds-checked dynamic load with an index of every integer type ---

checked_i8: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: i8{2}
    [Arr * Index] ! ret
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 30 =>

checked_i16: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: i16{2}
    [Arr * Index] ! ret
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 31 =>

checked_i32: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: i32{2}
    [Arr * Index] ! ret
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 32 =>

checked_i64: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: i64{2}
    [Arr * Index] ! ret
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 33 =>

checked_u8: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: u8{2}
    [Arr * Index] ! ret
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 34 =>

checked_u16: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: u16{2}
    [Arr * Index] ! ret
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 35 =>

checked_u32: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: u32{2}
    [Arr * Index] ! ret
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 36 =>

checked_u64: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: u64{2}
    [Arr * Index] ! ret
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 37 =>

checked_usize: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Index :: usize{2}
    [Arr * Index] ! ret
    V :: [Arr * Index unchecked]
    V isnt 12 -> _Exit 38 =>

// --- dynamic slicing with a begin index of every integer type ---

slice_i8: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Begin :: i8{1}
    Arr * Begin.. ! ret

slice_i16: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Begin :: i16{1}
    Arr * Begin.. ! ret

slice_i32: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Begin :: i32{1}
    Arr * Begin.. ! ret

slice_i64: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Begin :: i64{1}
    Arr * Begin.. ! ret

slice_u8: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Begin :: u8{1}
    Arr * Begin.. ! ret

slice_u16: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Begin :: u16{1}
    Arr * Begin.. ! ret

slice_u32: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Begin :: u32{1}
    Arr * Begin.. ! ret

slice_u64: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Begin :: u64{1}
    Arr * Begin.. ! ret

slice_usize: =>
    [Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Begin :: usize{1}
    Arr * Begin.. ! ret
