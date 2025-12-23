test.add: (i32, i32 =>i32, i32)
    +

libc.fopen: (addr c8, addr c8 =>addr FILE) // TODO main gets created here if test.add does not exist
    _fopen=>

// va2 :: 1, 2 test.add=>
// two :: 3, 4

file :: addr "easy.al"0, "r"0 _fopen=>
    is 0 -> "file open failed"0 _printf=>
buf :: addr 1024 _malloc=>
    is 0 -> "malloc failed"0 _printf=>

filelen :: i32 buf, 1, 1024, file _fread=>


va :: stack i64 filelen =[]
"filelen:\n %d\n"0, _printf=>

buf =[va]
"easy.al:\n %s\n"0, _printf=>

0
