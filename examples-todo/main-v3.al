filename: "todo.txt"

#enum i32 exit_codes {
    ok, invalid_usage, file_not_found, malloc_failed
}
#alias e exit_codes


file_open: (=>addr FILE file~, addr buf~, i64 file.len)
    file?~ :: main.filename, "r" fopen=>
    file? is null ->
        file_not_found:
            "file not found!" printf=> #e.file_not_found ret
    buf?~ :: 1024 malloc=>
    buf? is null ->
        "malloc failed!" printf=> #e.malloc_failed ret
    buf, #sizeof c8, 1024, file fread=> %
    file~>, buf~>, % ret

(i32 argc, addr addr |c8| argv =>i32)
argc is 1 ->

    f :: file_open=>
    ~ f.file~> fclose=>
    ~ f.buf~> free=>
    "%s\n", f.buf print=>
    e.ok ret

arg1 :: [argv, addr 1]]
is '-' not->
    usage:
        "whatever usage" printf=> #e.invalid_usage ret

cmd :: [arg1, c8]
cmd is 'c' ->
    create:
        file?~ :: filename, "a" fopen=>
        file? is null-> ->file_open.file_not_found
        ~ file~> fclose=>
        str :: [argv, addr c8 3]
        str strlen=> :: %
        str, #sizeof c8, %, file fwrite=>
        #e.ok ret

cmd is 'd' ->
    delete:
        file~, buf~, file.len :: file_open=>
        ~ file~> fclose=>
        ~ buf~> free=>
        file~ :: main.filename, "w" fopen=>
        ~ file~> fclose=>

        ptr :: buf

        skip_entry: subroutine
            [ptr, c8 ++] is '\n' or '\n' ->
                -> skip_entry

        target_idx :: [argv, addr c8 3] atoi=>

        target_idx @times
            skip_entry=>


        buf, #sizeof c8, ptr - buf, file fwrite=>
        skip_entry=>

        ptr - f.buf %
        ptr, sizeof c8, file_len - %, f.file fwrite=>

        #e.ok ret
