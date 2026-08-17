#declare printf: Format addr u8 => Num_Printed i32
#declare _Exit: Status i32

// A `ret` at a function body's own indent used to end the body: whatever
// followed it, including a label that a branch jumps to, was parsed as
// top-level code. These return from the middle of a body and then carry on at
// a label, with the branch taken and not taken.

branch_taken =>
branch_not_taken =>
label_ret_value =>
loop_after_ret =>
nested_ret =>

printf "all label tests passed\n" =>
ret 0

branch_taken: =>
    V :: after_ret 0 =>
    V isnt 42 -> _Exit 100 =>

branch_not_taken: =>
    V :: after_ret 1 =>
    V isnt 1 -> _Exit 101 =>

after_ret: I i32 => i32
    I is 0 skip->
    ret 1
    skip:

    ret 42

// the label block returns a value of its own, which needs the enclosing
// function's return list

label_ret_value: =>
    V :: tail_value =>
    V isnt 7 -> _Exit 102 =>

tail_value: => i32
    Zero :: i32{0}
    Zero is 0 done->
    ret 1
    done:

    W :: i32{7}
    ret W

// a loop whose exit label sits after a ret

loop_after_ret: =>
    V :: counted =>
    V isnt 3 -> _Exit 103 =>

counted: => i32
    [Count] :: 0 =[]
    loop:
    [Count] + 1 =[Count]
    [Count] >= 3 loop.break->
    loop->
    ret 0
    loop.break:

    ret [Count]

// a ret in a nested block still leaves the outer body running

nested_ret: =>
    V :: nested 0 =>
    V isnt 5 -> _Exit 104 =>
    W :: nested 1 =>
    W isnt 9 -> _Exit 105 =>

nested: I i32 => i32
    I is 0 ->
        ret 5
    ret 9
