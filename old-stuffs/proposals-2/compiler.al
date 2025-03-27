main:
    "main.al", "r" =>fopen
    =[file: addr]

    =>flen
    =[file.len: i32]

    =>malloc
    =[buf: addr]
    =:buf

    [file], buf, [file.len] =>fread

    1024
    =>malloc
    =[obuf: addr]

    ptr := buf
    :: loop
        defer ptr += 1
        ptr - buf
        ? [file.len]
        ge ->brk

        end := ptr
        find_end: loop
            defer end += 1
            [end u8]
            test ' ' or '\n' or '\0'
            eq ->brk
        '\0' =[end]

        [end - 1]
        if ':' ->
            ptr =>symtab.add
            ->brk.sw
        if '"' ->
            ptr =>strtab.add
            ->brk.sw
        brk.sw:
--
        # match
            if % ->
                %
                ->brk
            .
            ':', ptr =>symtab.add
            '"', ptr =>strtab.add

        match t {
            f => bla
            d => bla
        }
__

            ? ':' ne->foo
            ptr =>symtab.add
            ->brk.sw
        foo:
            ? '"' ne->brk.sw
            ptr =>strtab.add
        brk.sw:

__
        cmp w0, ':'
        bne foo
        mov w0, ptr
        bl symtab.add
        b brk.sw
    foo:
        cmp w0, '"'
        bne brk.sw
        mov w0, ptr 
        bl strtab.add
    brk.sw:

