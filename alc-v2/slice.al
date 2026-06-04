#compile_all expect.al

[Arr] :: 5*i32{.. 0} =[]

Arr..

Arr.1..

Arr..1

Arr..5

Arr.1..1

Slice :: Arr..

// dynamic

dynamic =>

ret 0

dynamic: =>
    [Arr] :: 5*i32{.. 0} =[]
    I :: 1

    Arr * I.. ! ret


