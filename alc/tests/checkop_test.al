#declare printf: Format addr u8 => Num_Printed i32
#declare _Exit: Status i32

// Exercises every form of the check operator `!`. The action after `!` is one
// of `ret`, `ret <value>`, `eret` or `<label>->`, and `unchecked` opts out.
// Each action is paired with every context that parses one: a static or
// dynamic index inside `[]`, a dynamic range, and a statement-level check of a
// check-typed value. Both outcomes (check taken and not taken) are covered.

[Arr] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
Slice :: Arr..

index_ret_value =>
index_ret_bare =>
index_branch =>
index_eret =>
index_unchecked =>

range_ret_value =>
range_ret_bare =>
range_branch =>
range_eret =>

slice_range Slice =>
slice_elem Slice =>
store_elem =>

value_ret_value =>
value_ret_bare =>
value_branch =>
value_eret =>

printf "all check operator tests passed\n" =>
ret 0

// --- dynamic index inside `[]` ---

index_ret_value: =>
    A :: index_at 2 =>
    A isnt 12 -> _Exit 100 =>
    B :: index_at 4 =>
    B isnt 14 -> _Exit 101 =>
    C :: index_at 5 =>
    C isnt 99 -> _Exit 102 =>
    D :: index_at 10 =>
    D isnt 99 -> _Exit 103 =>

index_at: I usize => i32
    [Buf] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    V :: [Buf * I ! ret 99]
    ret V

index_ret_bare: =>
    [Flag] :: 0 =[]
    index_reached Flag, 2 =>
    [Flag] isnt 1 -> _Exit 104 =>
    0 =[Flag]
    index_reached Flag, 10 =>
    [Flag] isnt 0 -> _Exit 105 =>

index_reached: F addr i32, I usize =>
    [Buf] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    [Buf * I ! ret]
    i32{1} =[F]

index_branch: =>
    index_branch_at 2, 1 =>
    index_branch_at 10, 0 =>

index_branch_at: I usize, Expected i32 =>
    [Buf] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    [Flag] :: 0 =[]
    loop:
    [Buf * I ! loop.break->]
    i32{1} =[Flag]
    loop.break:

    [Flag] isnt Expected -> _Exit 106 =>

index_eret: =>
    A :: index_eret_caller 2 =>
    A isnt 12 -> _Exit 107 =>
    B :: index_eret_caller 10 =>
    B isnt 55 -> _Exit 108 =>

index_eret_caller: I usize => i32
    V :: index_eret_at I =>
    V ! ret 55
    ret V

index_eret_at: I usize => !9 i32
    [Buf] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    V :: [Buf * I ! eret]
    ret V

index_unchecked: =>
    [Buf] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    I :: usize{2}
    V :: [Buf * I unchecked]
    V isnt 12 -> _Exit 109 =>

// --- dynamic range ---

range_ret_value: =>
    range_ret_value_begin =>
    range_ret_value_end =>
    range_ret_value_both =>

range_ret_value_begin: =>
    Four :: usize{4}
    A :: range_begin_len 1 =>
    A isnt Four -> _Exit 110 =>
    B :: range_begin_len 10 =>
    B isnt 99 -> _Exit 111 =>

range_ret_value_end: =>
    Three :: usize{3}
    A :: range_end_len 3 =>
    A isnt Three -> _Exit 112 =>
    B :: range_end_len 10 =>
    B isnt 99 -> _Exit 113 =>

range_ret_value_both: =>
    Two :: usize{2}
    A :: range_both_len 1, 3 =>
    A isnt Two -> _Exit 114 =>
    B :: range_both_len 1, 10 =>
    B isnt 99 -> _Exit 115 =>

range_begin_len: I usize => usize
    [Buf] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Sub :: Buf * I.. ! ret 99
    ret Sub.Length

range_end_len: I usize => usize
    [Buf] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Sub :: Buf * ..I ! ret 99
    ret Sub.Length

range_both_len: I usize, J usize => usize
    [Buf] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Sub :: Buf * I..J ! ret 99
    ret Sub.Length

range_ret_bare: =>
    [Flag] :: 0 =[]
    range_reached Flag, 1 =>
    [Flag] isnt 1 -> _Exit 116 =>
    0 =[Flag]
    range_reached Flag, 10 =>
    [Flag] isnt 0 -> _Exit 117 =>
    0 =[Flag]
    range_order_reached Flag, 1, 3 =>
    [Flag] isnt 1 -> _Exit 118 =>
    0 =[Flag]
    range_order_reached Flag, 3, 1 =>
    [Flag] isnt 0 -> _Exit 119 =>

range_reached: F addr i32, I usize =>
    [Buf] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Buf * I.. ! ret
    i32{1} =[F]

range_order_reached: F addr i32, I usize, J usize =>
    [Buf] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Buf * I..J ! ret
    i32{1} =[F]

range_branch: =>
    range_branch_at 1, 1 =>
    range_branch_at 10, 0 =>

range_branch_at: I usize, Expected i32 =>
    [Buf] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    [Flag] :: 0 =[]
    loop:
    Buf * I.. ! loop.break->
    i32{1} =[Flag]
    loop.break:

    [Flag] isnt Expected -> _Exit 120 =>

range_eret: =>
    A :: range_eret_caller 1 =>
    A isnt 3 -> _Exit 121 =>
    B :: range_eret_caller 10 =>
    B isnt 55 -> _Exit 122 =>

range_eret_caller: I usize => i32
    V :: range_eret_at I =>
    V ! ret 55
    ret V

range_eret_at: I usize => !9 i32
    [Buf] :: 5*i32{.0 10 .1 11 .2 12 .3 13 .4 14} =[]
    Buf * I.. ! eret
    ret 3

// --- dynamic range over a slice parameter ---

slice_range: S slice i32 =>
    Four :: usize{4}
    A :: slice_range_len S, 1 =>
    A isnt Four -> _Exit 123 =>
    B :: slice_range_len S, 10 =>
    B isnt 99 -> _Exit 124 =>

    slice_range_branch_at S, 1, 1 =>
    slice_range_branch_at S, 10, 0 =>

    C :: slice_range_eret_caller S, 10 =>
    C isnt 55 -> _Exit 125 =>

slice_range_len: S slice i32, I usize => usize
    Sub :: S * I.. ! ret 99
    ret Sub.Length

slice_range_branch_at: S slice i32, I usize, Expected i32 =>
    [Flag] :: 0 =[]
    loop:
    S * I.. ! loop.break->
    i32{1} =[Flag]
    loop.break:

    [Flag] isnt Expected -> _Exit 126 =>

slice_range_eret_caller: S slice i32, I usize => i32
    V :: slice_range_eret_at S, I =>
    V ! ret 55
    ret V

slice_range_eret_at: S slice i32, I usize => !9 i32
    S * I.. ! eret
    ret 3

// --- statement-level check of a check-typed value ---

value_ret_value: =>
    A :: value_ret_value_at 3 =>
    A isnt 3 -> _Exit 127 =>
    B :: value_ret_value_at 9 =>
    B isnt 55 -> _Exit 128 =>

value_ret_value_at: I i32 => i32
    V :: tagged I =>
    V ! ret 55
    ret V

value_ret_bare: =>
    [Flag] :: 0 =[]
    value_reached Flag, 3 =>
    [Flag] isnt 1 -> _Exit 129 =>
    0 =[Flag]
    value_reached Flag, 9 =>
    [Flag] isnt 0 -> _Exit 130 =>

value_reached: F addr i32, I i32 =>
    V :: tagged I =>
    V ! ret
    i32{1} =[F]

value_branch: =>
    value_branch_at 3, 1 =>
    value_branch_at 9, 0 =>

value_branch_at: I i32, Expected i32 =>
    [Flag] :: 0 =[]
    V :: tagged I =>
    loop:
    V ! loop.break->
    i32{1} =[Flag]
    loop.break:

    [Flag] isnt Expected -> _Exit 131 =>

value_eret: =>
    A :: value_eret_caller 3 =>
    A isnt 3 -> _Exit 132 =>
    B :: value_eret_caller 9 =>
    B isnt 55 -> _Exit 133 =>

value_eret_caller: I i32 => i32
    V :: value_eret_at I =>
    V ! ret 55
    ret V

value_eret_at: I i32 => !9 i32
    V :: tagged I =>
    V ! eret
    ret V

tagged: I i32 => !9 i32
    ret I

// --- element access on a slice, static and dynamic index ---

slice_elem: S slice i32 =>
    slice_elem_static S =>
    slice_elem_dynamic S =>
    slice_elem_ret_bare S =>
    slice_elem_branch S =>
    slice_elem_eret S =>
    slice_elem_unchecked S =>
    slice_elem_store =>
    slice_elem_store_dynamic =>

slice_elem_static: S slice i32 =>
    A :: slice_static_first S =>
    A isnt 10 -> _Exit 134 =>
    B :: slice_static_third S =>
    B isnt 13 -> _Exit 135 =>
    C :: slice_static_past_end S =>
    C isnt 99 -> _Exit 136 =>

slice_static_first: S slice i32 => i32
    V :: [S.0 ! ret 99]
    ret V

slice_static_third: S slice i32 => i32
    V :: [S.3 ! ret 99]
    ret V

slice_static_past_end: S slice i32 => i32
    End :: usize{2}
    Sub :: S * ..End ! ret 98
    V :: [Sub.3 ! ret 99]
    ret V

slice_elem_dynamic: S slice i32 =>
    A :: slice_dyn_at S, 2 =>
    A isnt 12 -> _Exit 137 =>
    B :: slice_dyn_at S, 4 =>
    B isnt 14 -> _Exit 138 =>
    C :: slice_dyn_at S, 5 =>
    C isnt 99 -> _Exit 139 =>
    D :: slice_dyn_at S, 10 =>
    D isnt 99 -> _Exit 140 =>

slice_dyn_at: S slice i32, I usize => i32
    V :: [S * I ! ret 99]
    ret V

slice_elem_ret_bare: S slice i32 =>
    [Flag] :: 0 =[]
    slice_elem_reached Flag, S, 2 =>
    [Flag] isnt 1 -> _Exit 141 =>
    0 =[Flag]
    slice_elem_reached Flag, S, 10 =>
    [Flag] isnt 0 -> _Exit 142 =>

slice_elem_reached: F addr i32, S slice i32, I usize =>
    [S * I ! ret]
    i32{1} =[F]

slice_elem_branch: S slice i32 =>
    slice_elem_branch_at S, 2, 1 =>
    slice_elem_branch_at S, 10, 0 =>

slice_elem_branch_at: S slice i32, I usize, Expected i32 =>
    [Flag] :: 0 =[]
    loop:
    [S * I ! loop.break->]
    i32{1} =[Flag]
    loop.break:

    [Flag] isnt Expected -> _Exit 143 =>

slice_elem_eret: S slice i32 =>
    A :: slice_elem_eret_caller S, 2 =>
    A isnt 12 -> _Exit 144 =>
    B :: slice_elem_eret_caller S, 10 =>
    B isnt 55 -> _Exit 145 =>

slice_elem_eret_caller: S slice i32, I usize => i32
    V :: slice_elem_eret_at S, I =>
    V ! ret 55
    ret V

slice_elem_eret_at: S slice i32, I usize => !9 i32
    V :: [S * I ! eret]
    ret V

slice_elem_unchecked: S slice i32 =>
    I :: usize{2}
    A :: [S * I unchecked]
    A isnt 12 -> _Exit 146 =>
    B :: [S.4 unchecked]
    B isnt 14 -> _Exit 147 =>

// --- static-index store through a slice ---

slice_elem_store: =>
    [Buf] :: 5*i32{.. 0} =[]
    S :: Buf..
    i32{7} =[S.3 ! ret]
    V :: [S.3 ! ret]
    V isnt 7 -> _Exit 148 =>

slice_elem_store_dynamic: =>
    [Buf] :: 5*i32{.. 0} =[]
    S :: Buf..
    I :: usize{2}
    i32{7} =[S * I ! ret]
    V :: [S * I ! ret]
    V isnt 7 -> _Exit 149 =>
    W :: [S.4 unchecked]
    W isnt 0 -> _Exit 150 =>

// --- store through a checked index ---

store_elem: =>
    store_elem_ret_value =>
    store_elem_ret_bare =>
    store_elem_branch =>
    store_elem_eret =>
    store_elem_unchecked =>
    store_elem_narrow =>

store_elem_ret_value: =>
    A :: store_at 2 =>
    A isnt 7 -> _Exit 151 =>
    B :: store_at 5 =>
    B isnt 99 -> _Exit 152 =>
    C :: store_at 10 =>
    C isnt 99 -> _Exit 153 =>

store_at: I usize => i32
    [Buf] :: 5*i32{.. 0} =[]
    i32{7} =[Buf * I ! ret 99]
    V :: [Buf * I ! ret 98]
    ret V

store_elem_ret_bare: =>
    [Flag] :: 0 =[]
    store_reached Flag, 2 =>
    [Flag] isnt 1 -> _Exit 154 =>
    0 =[Flag]
    store_reached Flag, 10 =>
    [Flag] isnt 0 -> _Exit 155 =>

store_reached: F addr i32, I usize =>
    [Buf] :: 5*i32{.. 0} =[]
    i32{7} =[Buf * I ! ret]
    i32{1} =[F]

store_elem_branch: =>
    store_branch_at 2, 1 =>
    store_branch_at 10, 0 =>

store_branch_at: I usize, Expected i32 =>
    [Buf] :: 5*i32{.. 0} =[]
    [Flag] :: 0 =[]
    loop:
    i32{7} =[Buf * I ! loop.break->]
    i32{1} =[Flag]
    loop.break:

    [Flag] isnt Expected -> _Exit 156 =>

store_elem_eret: =>
    A :: store_eret_caller 2 =>
    A isnt 3 -> _Exit 157 =>
    B :: store_eret_caller 10 =>
    B isnt 55 -> _Exit 158 =>

store_eret_caller: I usize => i32
    V :: store_eret_at I =>
    V ! ret 55
    ret V

store_eret_at: I usize => !9 i32
    [Buf] :: 5*i32{.. 0} =[]
    i32{7} =[Buf * I ! eret]
    ret 3

store_elem_unchecked: =>
    [Buf] :: 5*i32{.. 0} =[]
    I :: usize{3}
    i32{7} =[Buf * I unchecked]
    A :: [Buf * I unchecked]
    A isnt 7 -> _Exit 159 =>
    B :: [Buf.4]
    B isnt 0 -> _Exit 160 =>

store_elem_narrow: =>
    [Buf] :: 5*u8{.. 0} =[]
    I :: usize{2}
    u8{7} =[Buf * I ! ret]
    A :: [Buf * I ! ret]
    A isnt 7 -> _Exit 161 =>
    B :: [Buf.1]
    B isnt 0 -> _Exit 162 =>
