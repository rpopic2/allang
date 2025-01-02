fn main <- int:
    => argc: i32, argv: Span(Span(char))
    [argc], 2
    (!=) ?
        "please provide source file\n" -> print
        1; ret
    :

    [argv.data, 1] => [source_path: Span(char)]
    [source_path, data], "r" -> fs::open
    => [file: ptr(File)]
    (!) ?
        "source file does not exist\n" -> print
        2; ret
    :

    buf: Buffer -> Buffer::init
    , [header, data] -> Buffer::puts

    TokenType::Default
    = next_token: TokenType
    MAX_ALLOC_TOKEN
    -> alloc
    = token: Span_char
    0: usize
    = token_buf_idf
    '0' => reg

    0: i32
    ::= stack_size, stack_current_top (i32)
    ::= memory (4 bytes)
    4, (4 bytes) ::= stack_size_str: (char Span)
    8, (8 bytes) ::= itoa_buf: (char Span)

    comptime 0x10: int |> STACK_INC

    "stack sentry phase\n" -> print

    names: HashMap
    -> hashmap_new

    loop:
        source_file
        -> fs::getc
        ]c[, ((== '\n') = is_newline)

        != (' ', ',', is_newline, EOF)
        ?
            token


