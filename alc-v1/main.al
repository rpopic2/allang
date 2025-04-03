add: (i32, i32 =>i32, i32)
    +

"Hello World!\n"0 _printf=>
1, 0 add=>

va :: stack i64 0

// easy :: "easy.al", "r" _fopen=>
"main.al"0, "r"0 _fopen=>
=[va]
//easy =[va]

va


"Hello %p!\n"0, 2 _printf=>
