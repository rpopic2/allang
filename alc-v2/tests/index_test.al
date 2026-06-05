#declare printf: Format addr u8 => Num_Printed i32
#declare _Exit: Status i32

elem_i8 =>
elem_i16 =>
elem_i32 =>
elem_i64 =>
elem_u8 =>
elem_u16 =>
elem_u32 =>
elem_u64 =>

store_i8 =>
store_i16 =>
store_i32 =>
store_i64 =>
store_u8 =>
store_u16 =>
store_u32 =>
store_u64 =>

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

// --- static indexing of arrays of every integer element type ---
// (covers narrow i8/i16/u8/u16 elements, whose aggregate initialization
//  packs several elements into one register before storing.)

elem_i8: =>
    [Arr] :: 5*i8{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    A :: [Arr.0]
    A isnt 10 -> _Exit 10 =>
    B :: [Arr.2]
    B isnt 12 -> _Exit 10 =>
    C :: [Arr.4]
    C isnt 14 -> _Exit 10 =>

elem_i16: =>
    [Arr] :: 5*i16{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    A :: [Arr.0]
    A isnt 10 -> _Exit 11 =>
    B :: [Arr.2]
    B isnt 12 -> _Exit 11 =>
    C :: [Arr.4]
    C isnt 14 -> _Exit 11 =>

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

elem_u8: =>
    [Arr] :: 5*u8{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    A :: [Arr.0]
    A isnt 10 -> _Exit 14 =>
    B :: [Arr.2]
    B isnt 12 -> _Exit 14 =>
    C :: [Arr.4]
    C isnt 14 -> _Exit 14 =>

elem_u16: =>
    [Arr] :: 5*u16{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    A :: [Arr.0]
    A isnt 10 -> _Exit 15 =>
    B :: [Arr.2]
    B isnt 12 -> _Exit 15 =>
    C :: [Arr.4]
    C isnt 14 -> _Exit 15 =>

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

// --- static-index stores into arrays of every integer element type ---
// (each element is written through a distinct [Arr.N] target, then read back;
//  index 1 is left zero to confirm neighbouring elements are untouched.)

store_i8: =>
    [Arr] :: 5*i8{.. 0} =[]
    i8{10} =[Arr.0]
    i8{12} =[Arr.2]
    i8{14} =[Arr.4]
    A :: [Arr.0]
    A isnt 10 -> _Exit 50 =>
    B :: [Arr.2]
    B isnt 12 -> _Exit 50 =>
    C :: [Arr.4]
    C isnt 14 -> _Exit 50 =>
    Z :: [Arr.1]
    Z isnt 0 -> _Exit 50 =>

store_i16: =>
    [Arr] :: 5*i16{.. 0} =[]
    i16{10} =[Arr.0]
    i16{12} =[Arr.2]
    i16{14} =[Arr.4]
    A :: [Arr.0]
    A isnt 10 -> _Exit 51 =>
    B :: [Arr.2]
    B isnt 12 -> _Exit 51 =>
    C :: [Arr.4]
    C isnt 14 -> _Exit 51 =>
    Z :: [Arr.1]
    Z isnt 0 -> _Exit 51 =>

store_i32: =>
    [Arr] :: 5*i32{.. 0} =[]
    i32{10} =[Arr.0]
    i32{12} =[Arr.2]
    i32{14} =[Arr.4]
    A :: [Arr.0]
    A isnt 10 -> _Exit 52 =>
    B :: [Arr.2]
    B isnt 12 -> _Exit 52 =>
    C :: [Arr.4]
    C isnt 14 -> _Exit 52 =>
    Z :: [Arr.1]
    Z isnt 0 -> _Exit 52 =>

store_i64: =>
    [Arr] :: 5*i64{.. 0} =[]
    i64{10} =[Arr.0]
    i64{12} =[Arr.2]
    i64{14} =[Arr.4]
    A :: [Arr.0]
    A isnt 10 -> _Exit 53 =>
    B :: [Arr.2]
    B isnt 12 -> _Exit 53 =>
    C :: [Arr.4]
    C isnt 14 -> _Exit 53 =>
    Z :: [Arr.1]
    Z isnt 0 -> _Exit 53 =>

store_u8: =>
    [Arr] :: 5*u8{.. 0} =[]
    u8{10} =[Arr.0]
    u8{12} =[Arr.2]
    u8{14} =[Arr.4]
    A :: [Arr.0]
    A isnt 10 -> _Exit 54 =>
    B :: [Arr.2]
    B isnt 12 -> _Exit 54 =>
    C :: [Arr.4]
    C isnt 14 -> _Exit 54 =>
    Z :: [Arr.1]
    Z isnt 0 -> _Exit 54 =>

store_u16: =>
    [Arr] :: 5*u16{.. 0} =[]
    u16{10} =[Arr.0]
    u16{12} =[Arr.2]
    u16{14} =[Arr.4]
    A :: [Arr.0]
    A isnt 10 -> _Exit 55 =>
    B :: [Arr.2]
    B isnt 12 -> _Exit 55 =>
    C :: [Arr.4]
    C isnt 14 -> _Exit 55 =>
    Z :: [Arr.1]
    Z isnt 0 -> _Exit 55 =>

store_u32: =>
    [Arr] :: 5*u32{.. 0} =[]
    u32{10} =[Arr.0]
    u32{12} =[Arr.2]
    u32{14} =[Arr.4]
    A :: [Arr.0]
    A isnt 10 -> _Exit 56 =>
    B :: [Arr.2]
    B isnt 12 -> _Exit 56 =>
    C :: [Arr.4]
    C isnt 14 -> _Exit 56 =>
    Z :: [Arr.1]
    Z isnt 0 -> _Exit 56 =>

store_u64: =>
    [Arr] :: 5*u64{.. 0} =[]
    u64{10} =[Arr.0]
    u64{12} =[Arr.2]
    u64{14} =[Arr.4]
    A :: [Arr.0]
    A isnt 10 -> _Exit 57 =>
    B :: [Arr.2]
    B isnt 12 -> _Exit 57 =>
    C :: [Arr.4]
    C isnt 14 -> _Exit 57 =>
    Z :: [Arr.1]
    Z isnt 0 -> _Exit 57 =>

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
