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

Argc is 1 ->
    File :: addr file "todo.txt"0, "rw"0 _fopen=>
    Buffer :: addr void 1024 _malloc=>
    // File_Length :: i32 File filelen=>
    File_Length :: i32 Buffer, 1, 1024, File _fread=>
    File_Length =[Va]
    "%d\n"0 _printf=>

    Buffer =[Va]
    "todo.txt:\n %s\n"0, _printf=>

    File _fclose=>
    0 ret

0
