Condition ::
    Token. equals "is " ->
        <- COND_EQ
    Token. equals "isnt " ->
        <- COND_NE
    : <- COND_NV

Condition is COND_EQ ->
    "compare.." print=>

    #alias Reg = Register
    #alias SF = SignFlag
    Reg, SF ::
        Target_Nreg is pointing ->
            Reg[Target_Nreg.Reg]
            Target_Nreg nreg_sf=> %
            <- 0, %
        Target_Scratch is pointing ->
            Reg[Target_Scratch.Reg]
            Target_Scratch nreg_sf=> %
            <- 0, %

        Rewind ::
            Token.data as Iter c8
            Rewind(- 1 c8)
        [Rewind] #is_space @while
            Rewind(- 1 c8)
        [Rewind] is '>'
        and [Rewind(- 1 c8)] isnt '>' ?
            0 : 8
        <- , W

    Token. equals " 0 " ->
        @TokenStart
        @ReadToken
        @TokenEnd
        Token. not Equals "->" ->
            CompileErr "Compile Error in 0 branch\n"
            <<<-

        "..b.cond.." print=>
        Jump_Offset ::
            Find :: Stab_Loc, Token stab_search=>
            Find.Find isnt pointing ->
                Resolves. resolv {.Name Token, .Offset S.Code.Count}
                ls_add_resolv=>
                0 >>
            :
                Target :: [Stab_Loc.Data, Find.Symbolnum].N_Value
                S.Code.Count * #sizeof u32 %
                Target - %
            <<

            < 0 ->
                #define IMM19_MINUS2 = 0x1ffffb
                + IMM19_MINUS2

        S.Code, (SF, Reg, Jump_Offset)@cbz ls_add_u32=>
        <<--

    next=> c()
    @TokenStart
    @ReadToken
    @TokenEnd
    Is_Minus, Number ::
        _Number :: option.i32 Unset
        C is
        | '-' and [Token.Data, 1 c8] @is_num ->
            Is_Minus(true)
            next=> C()
            @TokenStart
            @ReadToken
            @TokenEnd
        | '\'' ->
            next=> C()
            _Number(C as i32)
            Next=> C()
            Next=> C()
        
        Token print=> 
        [Token.Data] @is_num ->
            Token.Data. to_i32=> Number()
            >>
        _Number is Unset ->
            "Compile Error: Number expected, was %d" print=>
        Number(_Number. @unwrap)
        <<

    S.Code, (SF, Reg, Number, Is_Minus)@cmp ls_add_u32=>

    Next=> C()
    @TokenStart
    @ReadToken
    @TokenEnd
    Token.Equals "->" ->
        `Syntax Error: -> expected, but found `C print=>

    Token.Len > 0 ->
        "branch to.." print=>
        Token print=>
        Resolves, resolv {.Name Token, .Offset S.Code.Count}
        ls_add_resolv=>
        S.Code, (0, COND_EQ)@b_cond ls_add_u32=>
        >>
    :
        "anonymous label.." print=>
        Tmp_To_Resolve(S.Code.Count)
        S.Code, (0, Condition)@b_cond ls_add_u32=>
    <<

    [It.Data] is '\n' ->
        Pending_Put_Label is true ->
            Pending_Lable_Ident(+ 4)
        >>
    Pending_Put_Label is true ->
        Tmp_Put_Label_Nextline(true)
    <<

    