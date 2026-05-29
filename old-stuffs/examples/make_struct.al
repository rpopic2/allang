#alias Dst :: Destination
#alias Memb :: Member
eightbyte_make_struct: $Dst addr reg_t, Type addr type_t, Args addr dyn.regable, Index addr &i32 => !
    Members :: {
        . [Type.Struct.Members]
        .Count Members .len @
    }

    Dst :: reg_t{
        .Rsize [Type.Size] .min 8
        .. [$Dst]
    } =[]

    &Size_Acc :: 0
    &Base_Offset :: 0 bits
    &Cleared :: !

    loop:
        #pre I :: [Index]
        I < Members.Count !
            break loop->
        #defer
            I :+ 1
            [Index] := {I}

        Memb :: {
            . [Members addr member_t*I]
            Type :: [Memb.Type type_t]
            .Size [Type.Size]
            .Offset [Memb.Offset] * 8 bits
        }

        Memb.Offset :-
            Size_Acc is 0 ?
                Base_Offset := Memb.Offset
            Base_Offset

        Size_Acc
        . :+ Memb.Size
        . > 8 ?
            . :- Memb.Size
            [Index] := {I}
            break loop->

        R :: [[Args addr regable * I] regable]
        R is {value, &Value} ?
            Memb.Offset mod 16 is 0 ?
                &Size :: Memb.Size
                while Size < 2 @
                    I :+ 1
                    I > Members.Count ?
                        break while->
                    R :: [Args.Begin] + I
                    [R.Tag] is Value !
                        I :- 1
                        break while->

                    Memb :: [Members addr member_t * I]
                    Memb.Size :: [Memb.Type].Size]
                    Size + Memb.Size > 2 ?
                        I :- 1
                        break while->
                    Size :+ Memb.Size
                    Value :orr ([R.Value] shl (Msmb_Size * 8))

            Value is 0
                loop->

            Memb.Offset mod 16 is 0 !
                Cleared ! unreachable
                emit_rri "orr", [Dst], Dst, (Value shl Memb.Offset) =>
                loop->
            :
                emit_risi Cleared ! "movz" : "movk",
                    Dst, Value, "lsl", offset =>
            Cleared := ?
            loop->

        R is {reg, &Reg} ?
            Memb.Offset is 0 ?
                Memb.Size < 4 ?
                    Mask ::
                        0xff
                        for I .. Memb.Size excl
                            Mask :or 0xff shl (I * 8)
                    Reg.Rsize .max Dst.Rsize
                    emit_rri "and", Dst, Reg, Mask =>
                :
                    Tmp_Dst ::
                        Dst
                        Tmp_Dst.Rsize .max Reg.Rsize
                    emit_rr "mov", Tmp_Dst, Reg =>
            :
                Reg.Rsize .max Dst.Rsize
                Width :: i64{Memb.Size} * 8
                emit_rrii (cleared ? "bfi" : "ubfiz"), Dst, Reg, {Memb.Offset}, Width =>
            Cleared := ?
            loop->
        unreachable

    [Size] :+ Size_Acc
    ret Cleared

#enum memberof *i32 :: X, Y, Z, W

Rect :: *{30, 20}
Rect.X
Rect.Y
