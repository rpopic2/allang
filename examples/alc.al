#alias std.file

src~ ::
    "main.al"
    file.read.tostr=> ~> $
    is Error ->
        FileError ret
    $

defer src.file ~> leak

parse=>

parse: (str src)
    parse_scope=>

parse_scope: (str src)
    `start parse` print=>
    token :: str noinit =[]
    it :: src into_iter=>

    named_reg_idx :: u8 19
    regoff :: int 0
    ident :: int 0
    target_nreg :: nreg ||

    s~ :: stack_context.new=>
    ~ s ~> stack_context.delete=>

    line_end :: addr c8 not pointing

    calls_fn :: false

    @inline next: sub (=>c8)
        c(it next=>)

    @loop

parse_scope.token:
    @inline start: sub ()
        =it.data[token.data]
    @inline end: sub ()
        it.data - token.data =[token.len]
    @inline read: sub ()
        c @is_alpha, or @is_num, or is '.', or is '_' @while
            @next
    @inline read_until_newline: sub ()
        c isnt '\n' and isnt '\0' @while
            @next

parse_scope.loop: sub
    c :: @next
    token_consumed :: false

    @token.start
    @token.read
    @token.end

    token.len is 0 and c is ' ' ->
        ++ident
        ->loop

    ident, 4 @mod.ne 0
        "Syntax error: single indentation should consist of 4 spaces"
        @compile_err

    line.end + 1 < it.data ->
        target_nreg : nreg ||
        addr c8 p :: it.data
        [p] isnt '\n' and isnt '\0' ->
            p +: c8 1
        line_end : p - c8 1

    $is_eof :: line_end[2] @at is '\0'
    -> token_consumed :: true

    `token '`token`,' (len: `token.len`, ident: `ident`, c: `c` `c as i32
    print=>

    @inline if_is: (str s, iter c8 it, addr i32 c =>bool)
        s, it, s.len
        libc.memcmp=>
        is 0 ->
            it, s.len .advance
            [it.data] =[c]
            true
        :
            false

    "//" @if_is ->
        `is comment` print=>
        @token.read_until_newline
        ->loop

    [it.data] is ':'
        @next
        c isnt ' ' and isnt '\n' ->
            "Syntax error: space or newline required after a label: "
            @compile_err

        @next
        `static label '`token`, `.. print=>

        f, token
        macho_stab_ext=>

        c is '(' ->
            `is a routine, has args `.. print=>
            @next
            @token.start
            @token.read
            @token.end

            read_type:
                "i32" @if_is ->
                    `type of i32 `..
                "c8" @if_is ->
                    `type of c8 `..
                "addr" @if_is ->
                    `type of addr `..
                    ->read_type
                print=>
            ", " @if_is ->
                ->read_type
            " =>" @if_is ->
                _>read_type

        @next   // )
        @next   // \n
        next_src :: { it.data }

        i, 0..src.len @range
            @next is '\n'
                c : [it.data, c8 1]
                c isnt ' ' and isnt '\n' <-

        ++depth
        `\n new stack of depth `depth print=>
        next_src.len : it.data - nextsrc.data

        parse_scope=>
        --depth

        `end of rt` print=>
        ->loop


