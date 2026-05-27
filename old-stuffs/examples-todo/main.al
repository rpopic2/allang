filename: "todo.txt"

globl
main: (argc: i32, argv: addr =>i32)

argc ? 1 eq->
list_all: () =>
    =>read_file { } : f
    f.file ? ze-> 2 ret
    ~ f.file =>fclose
    ~ f.buf =>free

    "%s", .buf =>printf
    0 ret

[argv, 2 addr]
:: arg1
[arg1 c8] ? "-" ne->usage
[arg1, 1 c8]
:: cmd

cmd ? 'c' eq->
create:
    filename, "a" =>fopen : file
    [argv, 2 addr] : item
    '\n' =[item - 1]
    item =>strlen : item.len
    file, item - 1, item.len + 1 =>write
    0 ret

cmd ? 'd' eq->
delete:
    =>main.list_all.read_file =:: f
    f.buf =: ptr
    [argv, 2 addr]
    => str.to.i32 =: idx

    find_line_end: (ptr: addr =>ptr: addr) loop
        [ptr c8 += 1]
        ? '\n' or '\0'
        eq ->ptr ret

    0 =: i
    :: loop
        i ? idx ge<-
        ~ i += 1
        =>find_line_end

    f.file, f.buf, ptr - f.buf =>write
    =>find_line_end

    f.file, ptr, f.file.len - ptr =>write
    0 ret

read_file: (=> file: addr~?, buf: addr~, file.len: i32)
    filename, "r", =>fopen
    :: file
    1024 =>malloc
    :: buf
    null-> :: null, null ret
    =>filelen
    :: file.len
    file, buf, file.len =>fread
    file, buf, file.len ret
