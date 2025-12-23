I :: i32 0
Va :: stack i32 0 =[]

loop:
    I is 10 break->
    I =[Va]
    "%d\n"0 _printf=>
    I++
    loop->
break:
0

