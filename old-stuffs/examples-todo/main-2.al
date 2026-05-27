filename:: "todo.txt"

@enum exit_codes {
    ok, invalid_usage, file_not_found, malloc_failed
}
@alias exit_codes = e

ext
fopen: (usiz c8 filename, usiz c8 openmode => usiz FILE?~)

globl
main: (i32 argc, addr addr c8 argv => i32)
argc == 1
    file_open: (=> usiz FILE file~, usiz buf, i64 file.len)
        file :: filename, "r" fopen=>
        // ~ file fclose=> ; this should be a compile error
        file == null
            file_not_found:
                "file not found!" printf=> @e.file_not_found panic
        buf :: 1024 malloc=>
        buf == null
            "malloc failed!" printf=> @e.malloc_failed panic
        buf, sizeof c8, 1024, file fread=> =file.len
        file, buf, _ ret
    ~ f.file fclose=>
    f :: file_open=>
    "%s\n", f.buf= printf=>
    0 ret

arg1 :: [argv, 1]
[] != '-'
    usage:
        "whatever test usage" printf=> @e.invalid_usage ret

cmd :: [arg1, 2]
cmd == 'c'
    create:
        file :: filename, "a" fopen=>
        ~ file fclose=>
        file ze->main.file_not_found
        str :: [argv, 3]
        str strlen=> :: $2
        str, sizeof c8, $2, file fwrite=>
        0 ret

cmd == 'd'
    delete:
        f :: =>main.file_open
        ~f.file fclose=>
        // deleting the line above would be a compile error as...
        f.file :: filename, "w" fopen=> // this would be something like f.file.2
        // this is a shadowing.
        ~f.file fclose=>
        |f|

        ptr :: f.buf
        skip_entry: sub ()
            [ptr, c8 += 1] != '\n' @or '\0
                ->skip_entry
        target_idx :: [argv, 3] atoi=>

        _: target_idx @for
            ->>skip_entry

        f.buf, sizeof c8, ptr - f.buf, f.file fwrite=>

        ->>skip_entry

        cur :: (ptr - f.buf)
        ptr, sizeof c8, file_len - cur, f.file fwrite=>

        0 ret
