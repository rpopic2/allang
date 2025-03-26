"todo.al" ::: filename

(i32: argc, addr: argv) :main
    argc ? 0 eq->
    list_all:
        open_file: (=> addr :file~, i32 :file.len, addr :buf~)
            filename, "r" =>fopen : file

            1024 =>malloc : buf

            ., sizeof c8, 1024, file =>fread : file.len
            file, file.len, buf ret

        =>open_file : #f
        ~ f.file =>fclose
        ~ f.buf =>free
        "%s", f.buf =>printf
        0 ret

    [argv, 1 addr : arg1]
    [arg1, c8] ? '-' ne->usage

    [arg1, 1 c8 : cmd]
    cmd ? 'c'
    create:
        filename, "a" =>fopen : file
        [main.argv, 2 addr : entry]
        entry =>strlen : entry.len
        entry, sizeof c8, entry.len, file
        =>fwrite
        0 ret

    cmd ? 'd'
    delete:
        =>open_file # :: f
        ~ f.file =>fclose
        ~ f.buf =>free
        [argv, 2 addr] =>str.to.i32 : idx

        f.buf : ptr

        read_until_end:
            loop:
                [ptr, c8 += 1] ? '\n' #or '\0'
                ne->loop
            ret


usage:
    "usage:
    todo -c 'buy potatos'   create new item at the end
    todo -d 2               delete at index 2
    " =>printf
    0 =>exit
