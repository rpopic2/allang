struct file {

}
struct void { }

filelen: (addr file File =>i32 Retval)
    File, 0, 2 _fseek=>
    Length :: i32 File _ftell=>
    File, 0, 0 _fseek=>
    Length

(i32 Argc, addr addr c8 Argv)

Va :: stack i32 0 =[]

Argc =[Va]
"argc: %d\n"0 _printf=>

// Argc is 1 ->
    // File :: addr file "todo.txt"0, "rw"0 _fopen=>

    // File_Length :: i32 File filelen=>
    // File_Length =[Va]
    // "file len: %d\n"0 _printf=>

    // Buffer :: addr void File_Length _malloc=>
    // Buffer, 1, 1024, File
    // _fread=>

    // Buffer =[Va]
    // "todo.txt:\n%s\n"0, _printf=>

    // Buffer _free=>
    // File _fclose=>
    // 0 ret

[Argv] =[Va]
"first arg: %s\n"0 _printf=>

Arg1 :: addr c8 [Argv, 1 addr]
Arg1 =[Va]
"second arg: %s\n"0 _printf=>

Cmd :: c8 [Arg1, 1 c8]
Cmd =[Va]
"command: %c\n"0 _printf=>
// Cmd is 'c' ->
    // "create\n"0 _printf=>
    // File :: addr file "todo.txt"0, "a"0 _fopen=>
    // Item :: addr c8 [Argv, 2 addr]
    // Item_Length :: i32 Item _strlen=>
    // Item, 1, Item_Length, File _fwrite=>
    // "\n", 1, Item_Length, File _fwrite=>

Cmd is 'd' ->
    "delete\n"0 _printf=>
    File :: addr file "todo.txt"0, "r+"0 _fopen=>
    File_Length :: i32 File filelen=>
    Buffer :: addr void File_Length _malloc=>
    Buffer, 1, 1024, File
    _fread=>

    Arg2 :: addr c8 [Argv, 2 addr]
    Index :: i64 Arg2 _atoi=>
    continue:
        "hi" _printf=>
        continue->

0