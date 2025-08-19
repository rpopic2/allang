fib: (i64 N =>i64 Retval)
    N < 2 ->
        rett->
    N1 :: i64 N - 1 fib=>
    foo:
    N2 :: i64 N - 2 fib=>
    N1 + N2
    rett:
    0

Va :: stack i64 0 =[]

4 fib=> =[Va] "fib: %d\n"0 _printf=>

0
