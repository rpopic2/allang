// https://gobyexample.com/recursion

fact: (i32 n =>i32)
    n == 0 ->
        1 ret
    f - 1 fact=>
    * n ret

7 fact=> print=>

fib: (i32 n =>i32)
    n < 2 ->
        n ret
    a :: n - 1 fib=>
    b :: n - 2 fib=>
    a + b ret

7 fib=> print=>
