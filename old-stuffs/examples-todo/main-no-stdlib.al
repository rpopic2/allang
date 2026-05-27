|addr c8| filename: "todo.al"

i32 @std.array.instantiate

std.
globl
main: (@.str @.array args)
    args.len == 1
    print_all:
        file :: filename .file.open.read=>
        buf :: file.len .alloc.heap=>
        buf, file .file.read=>
        buf .io.print=>
        0 ret

    args.len != 2
        ->usage

    arg1 :: args, 1, usage @-array.at
    arg1, 1, usage @-str.at
    == '-' ->usage

    std.
    cmd :: arg1, 2, usage @.str.at
    cmd == 'c'
    create:
        file :: filename .file.open.append=>
        entry :: args, 1, usage @.array.at
        entry, file .file.write=>
        0 ret

    std.
    cmd == 'd'
    delete:
        file :: filename .file.open.=>
        del_index :: args, 2, usage @.array.at .str.to.i32=>
        toks :: "\0\n" .str.tok.new=>
        @loop i : 0
            i >= toks.len
                <-
            ~ i += 1
            i == del_index
                ->continue
            i, toks @.str.tok.at .file.write=>
        0 ret

