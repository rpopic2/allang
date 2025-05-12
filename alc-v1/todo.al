// put_todo: (int n, str todo)
// 	n: todo print=>

add: (i32, i32 =>i32)
    +

// struct point {
//     i32 x,
//     i32 y,
// }

va :: addr i32 0 =[]
vop :: addr c8 0 =[]
vb :: addr i32 0 =[]

a :: i32 0 =[]
b :: i32 0 =[]
op :: c8 0 =[]

_loop:
    a =[va]
    b =[vb]
    op =[vop]
    "%d %c %d"0, _scanf=>
        is -1 _break->

    [op]
    is '+' -> [a], [b] + >>
    : _loop->
    <<


    =[va]
    "%d\n"0, _printf=>

    _loop->

    _break:

0
