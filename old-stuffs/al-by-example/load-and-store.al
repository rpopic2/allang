load_store: (S str, *Context addr parser_context => Offset i32)
    #alias Cur_Token :: Context.Cur_Token
    @inline no_closing_bracket: (Token str => bool);  [Token.End, -1] isnt ']'@

    Offset = 0

    *S.Data += 1

    @define no_offset :: [S.End, -1] is ']'@
    @if @no_offset@@
        *S.End -= 1
        >>

    @define has_ofset :: [S.End] is ',' @
    @if @has_offset@@
        lex *Context =>
        ^*Offset +=
            read_regable [Cur_Token.Id], Cur_Token => .
            .Tag is Value ->
                compile_err Cur_token, `valid offset expected, but found `[Cur_Token.Id]`\n` =>
                >>
            i32(.Value)
        <<
        @if Cur_Token @.no_closing_bracket @@
            compile_err Cur_Token, `closing ']' expected\n`
    @elif S @.no_closing_bracket@@
        compile_err Cur_Token, `closing ']' expected\n`

    Self_Offset ::
        read_regable S, Cur_Token => .
        [.Tag] == Reg ->
            compile_err Cur_Token, "register required\n"
            ret 0
        = .Reg.Offset

    Offset *= #sizeof i32#
    Offset += Self_Offset

    ret Offset

