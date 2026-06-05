#declare _Exit: Status i32

Hello: 1
World :: 2

// this is done at runtime
World isnt 2 ->
    _Exit 2 =>

// constant folding happens with 'isnt', skipping codegen
// Hello isnt 1 ->
    // I :: 3
    // _Exit 1 =>

Hello is 1 -> nop =>

Hello isnt 2 ->
    nop =>

consume Hello is 1 =>
consume Hello isnt 1 =>

// named branch is broken at the moment
// Hello is 1 foo->

// foo:
    // ret 0


ret 0

nop: =>
    ret

consume: X i32 =>
    ret
