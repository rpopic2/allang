struct file {

}
struct void { }

filelen: (addr file File =>i32 Retval)
    File, 0, 2 _fseek=>
    Length :: i32 File _ftell=>
    File, 0, 0 _fseek=>
    Length

Va :: stack i32 0 =[]

File :: addr file "todo.txt"0, "rw"0 _fopen=>
Buffer :: addr void 1024 _malloc=>
File_Length :: i32 File filelen=>
File_Length =[Va]
"%d\n"0 _printf=>

File _fclose=>

0
