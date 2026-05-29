// https://gobyexample.com/recursion

fact: (n i32 =>i32)
    n is zero ->
        1 ret
    f - 1 fact=>
    * n ret

7 fact=> print=>

fib: (n i32 =>i32)
    n < 2 ->
        n ret
    a :: n - 1 fib=>
    b :: n - 2 fib=>
    a + b ret

7 fib=> print=>
