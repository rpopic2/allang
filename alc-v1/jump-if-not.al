(i32 Argc, addr addr c8 Argv)

Arg1 :: addr c8 [Argv, 1 addr]
Cmd :: c8 [Arg1, 1 c8]
Cmd is 'd' ->
    "was d\n"0 _printf=>
    Test :: i32 123

"done\n"0 _printf=>
0

