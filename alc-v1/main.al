test.add: (i32, i32 =>i32, i32)
    +

libc.fopen: (addr c8, addr c8 =>addr FILE)
    _fopen=>

va :: 1, 2 test.add=>
two :: 3, 4

file :: addr "easy.al"0, "r"0 libc.fopen=>
is 0 -> "file open failed"0 _printf=>
buf :: addr 1024 _malloc=>

filelen :: buf, 1, 1024, file _fread=>

// va :: stack i64 0
// buf =[va]

"hi %d\n"0, _printf=>

0

