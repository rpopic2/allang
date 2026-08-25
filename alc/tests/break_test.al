#declare printf: Format addr u8 => Num_Printed i32
#declare _Exit: Status i32

// A block's break label is generated where the block ends, and only when a
// branch targets it. `ret` is a break out of the function's own block.

named_block =>
nested_block =>
fn_break =>
explicit_break =>

printf "all break tests passed\n" =>
ret 0

// a label that opens an indented block owns <name>.break

named_block: =>
    V :: count_to 3 =>
    V isnt 3 -> _Exit 130 =>

count_to: Limit i32 => i32
    [Count] :: 0 =[]
    loop:
        [Count] + 1 =[Count]
        [Count] >= Limit loop.break->
        loop->
    ret [Count]

// an inner block breaks out of an outer one

nested_block: =>
    V :: sum_grid =>
    V isnt 21 -> _Exit 131 =>

sum_grid: => i32
    [Sum] :: 0 =[]
    outer:
        [Sum] + 10 =[Sum]
        inner:
            [Sum] + 1 =[Sum]
            [Sum] >= 21 outer.break->
            inner->
        outer->
    ret [Sum]

// <fn_name>.break-> lands where ret lands

fn_break: =>
    [Flag] :: 0 =[]
    set_unless Flag, 0 =>
    [Flag] isnt 5 -> _Exit 132 =>
    set_unless Flag, 1 =>
    [Flag] isnt 9 -> _Exit 133 =>

set_unless: F addr i32, I i32 =>
    i32{5} =[F]
    I is 0 set_unless.break->
    i32{9} =[F]

// an explicit <name>.break label suppresses the generated one

explicit_break: =>
    V :: first_hit =>
    V isnt 102 -> _Exit 134 =>

first_hit: => i32
    [Count] :: 0 =[]
    loop:
        [Count] + 1 =[Count]
        [Count] >= 2 loop.break->
        loop->
        loop.break:
        [Count] + 100 =[Count]
    ret [Count]
