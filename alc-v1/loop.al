_loop:
    va_0 :: addr i32 0 =[]

    buf :: i32 0 =[]

    buf =[va_0]

    [buf] =[va_0]
    "was %d\n"0, _printf=>
    is 0 _break->
    _loop->
    0
    _break:

0
