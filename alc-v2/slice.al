#compile_all expect.al

[Arr] :: 5*i32{.. 0} =[]

Arr..

Arr.1..

Arr..1

Arr..5

Arr.1..1

Slice :: Arr..

I :: 1
J :: 2

// dynamic

dynamic =>

ret 0

dynamic: =>
    [Arr] :: 5*i32{.. 0} =[]
    I :: 1
    J :: 2

    S1 :: Arr * I.. ! ret

    S2 :: Arr * ..I ! ret

    Arr * I..J ! ret

