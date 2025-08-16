struct file {

}
struct void { }

_filelen: (addr file File =>i32 Retval)
    File, 0, 2 _fseek=>
    Length :: i32 File _ftell=>
    File, 0, 0 _fseek=>
    Length

// test_fn: (addr c8 Iter =>addr c8)
//     +
//
// test_fn: (addr c8 Iter =>addr c8)


(i32 Argc, addr addr c8 Argv)

Va :: stack i64 0 =[]

Argc =[Va]
"argc: %d\n"0 _printf=>

// Argc is 1 ->
//     File :: addr file "todo.txt"0, "rw"0 _fopen=>
//
//     File_Length :: i32 File _filelen=>
//     File_Length =[Va]
//     "file len: %d\n"0 _printf=>
//
//     Buffer :: addr void File_Length _malloc=>
//     Buffer, 1, 1024, File
//     _fread=>
//
//     Buffer =[Va]
//     "todo.txt:\n%s\n"0, _printf=>
//
//     Buffer _free=>
//     File _fclose=>
//     0 ret

[Argv] =[Va]
"first arg: %s\n"0 _printf=>

Arg1 :: addr c8 [Argv, 1 addr]
Arg1 =[Va]
"second arg: %s\n"0 _printf=>

Cmd :: c8 [Arg1, 1 c8]
Cmd =[Va]
"command: %c\n"0 _printf=>
// Cmd is 'c' ->
//     "create\n"0 _printf=>
//     File :: addr file "todo.txt"0, "a"0 _fopen=>
//     Item :: addr c8 [Argv, 2 addr]
//     Item_Length :: i32 Item _strlen=>
//     Item, 1, Item_Length, File _fwrite=>
//     "\n", 1, 1, File _fwrite=>

Cmd is 'd' ->
    "delete\n"0 _printf=>
    File :: addr file "todo.txt"0, "r+"0 _fopen=>
    File_Length :: i32 File _filelen=> =[]
    [File_Length] =[Va]
    "filelen: %ld bytes\n"0 _printf=>
    Buffer :: addr void [File_Length] _malloc=>
    Buffer, 1, 1024, File
    _fread=>
    File _fclose=>

    Index :: i64 [Argv, 2 addr] _atoi=>
    Index =[Va]
    "requested index: %d\n"0 _printf=>

    Iter :: addr c8 Buffer

    I :: i32 0
    _loop:
        I is Index _break->
        _loop2:
            Iter++
            [Iter, 0 c8] isnt '\n' _loop2->
        I++
        _loop->
    _break:
    Iter++

    File :: addr file "todo.txt"0, "w"0 _fopen=>

    Iter - Buffer =[Va]
    "write %ld bytes\n"0 _printf=>
    Buffer, 1, Iter - Buffer, File _fwrite=>

    _loop3:
        Iter++
        [Iter, 0 c8] isnt '\n' _loop3->
    Iter++

    Write_Length :: i64 Iter - Buffer
    Write_Length =[Va]
    "write2 %ld bytes\n"0 _printf=>

    Iter, 1, Write_Length, File _fwrite=>

    File _fclose=>

    "ok2!\n"0 _printf=>
0
