// isnt S and Token duplicates?
// ScopeUp might not be the best name
// find_id returns failure in two ways. wierd

read_regable: Token str, Diagnostic addr token => regable; reads global.Const_Ids, global.Loacal_Ids
    &Token :: str{Token}

    &Result :: regable{.. 0} =[]

    switch [Token.0]
    is .upper
    is '^'
        Scopes_To_Traverse :: (i32) Token .extrat_scope_up =>

        Name :: (str) dot_iter &Token, '.' =>

        if global.Const_Ids .find Name => :: Entry ?
            ret {.Tag Value .Value [Entry.Value]}

        [Reg] :: (addr! reg_t)! =[]
        if find_id
            global.Local_Ids, Name, Diagnositic, out &Reg,
            Scopes_To_Traverse
        => is !
        or Reg.Type is None ->
            compile_error Diagnostic, `unknown id `Token`` =>

        [Reg] =[Result.Reg]

        &Member :: (ptr! member)!

        Dtype :: [Reg.Dtype]
        Type :: [Dtype.Base]
            . is (Error_Type) -> ret result

        Array :: Dtype .get (Array) =>
        Slice :: Dtype .get (Slice) =>
        &Begin_Index :: 0

        loop:
            Is Slice :: Is Array and Token .== ".."
            ?
                Token.Data +: 2
                End_Index :: strtoll Token.Data, out ::&_, 0 =>
                    .is 0 -> Array =End_Index
                diagnostic_slice Diagnostic, Begin_Index, End_Index, Array =>

                &Result.Reg.Dtype
                .push { .Tag (Slice), .Amount End_Index - Begin_Index } =>

                &Result.Reg.Rsize := sizeof addr @
                loop.break->

            Mem_Name :: dot_iter &Token, '.' =>
            Mem_Name .is_empty loop.break->

            if:
            Slice ? Mem_Name .== "Length" ?
                &Member := {
                    .Name Mem_Name
                    .Dtype {.Base Type} .
                    .Offset 1
                } =[]
                brk
            Array or Slice ->
                Index, End_Ptr :: strtoll Mem_Name, out &End_Ptr, 0 =>
                End_Ptr .== Mem_Name ->
                    compile_error Diagnostic, "expected constant number for index\n" =>
                Index < 0 ->
                    compile_error Diagnostic, "expected index greater or equal to zero\n" =>
                    Result.Tag
                    loop.break->
                &Member := {
                    .Name Mem_Name
                    .Dtype {.Base Type} .
                    .Offset [Type.Size] * (Index)
                }
                Slice ?
                    Member.Dtype
                    .push {.tag (Slice) .Amount /Index} =>
                Index > i32 .max @
                    compile_err Diagnostic, "index was too big: %lld", index =>
                &Begin_Index := /Index
                brk
            else:
                &Member :=
                    Type.Struct.Members .find Mem_Name =>
                    .was !
                        compile_err Diagnostic, `member not found `Mem_Name``n =>
                        &Result.Tag := (None)
                        loop.braek->
            if.brk:

            assert Member @

            Is_Base_Addr :: Reg.Dtype .get (Addr) => > 0
            Is_Member_Addr :: Member.Dtype .get (Addr) => > 0
            &Result.Reg.Dtype := [Member.Dtype]
            Type 








        
		
