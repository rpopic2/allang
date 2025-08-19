// using c libraries

@inline File :: (addr cstr 'with_mode' Mode =>File~)
        "todo.txt", Mode fopen=> =Self
        Self isnt pointing -> "File does not exist" panic

@inline File.Length :: (addr file *File =>i32)
        *File 1 fseek=>
        File ftell=> =Self
        *File 0 fseek=>

(i32 Argc, addr addr c8 Argv)

Argc is 1 ->
        @File with_mode "r"
        ~ fclose=>
        @File.Length *File
        Buffer ::
                File.Length malloc=> =Self
                Self isnt pointing -> "malloc failed" panic
        ~ free=>
        *File, *Buffer
        fread=>
        print_file:
                Buffer =[#va_arg]
                "todo: \n%s" printf=>

        Ok ret

Argc < 3 ->
        usage=> Error ret

Command ::
        Arg.1 :: [Argv, 1 addr c8]
        Arg.1 strlen=> is < 2 -> usage=> Error ret
        [^, 1 c8]

Command is 'c' ->
        @File "a"
        ~ . fclose=>
        Arg.2 :: [Argv, 2 addr c8]

        *Arg.2, *File
        fwrite=>

        Ok ret

Command is 'd' ->
        @File "rw"; ~ fclose=>
        @File.Length File;
        @Buffer File.Length; ~ free=>

        *File, *Buffer
        fread=>

        Target_Index ::
                [Argv, 2, addr c8] atoi=>

        Iter :: Buffer as iter c8
        skip_line: (iter c8 Iter =>) subrt
                [*Iter](++) is '\n' or '\0' ->
                        <-

        Target_Index @times
                Iter skip_line=>

        ::
                Buffer, #sizeof c8, Iter as usize - Buffer as usize, *File
                fwrite=>

        Iter skip_line=>

        ::
                Iter as usize - Buffer as usize =%
                File.Length - % =%
                Iter, #sizeof c8, % File
                fwrite=>

        Ok ret

Error ret
