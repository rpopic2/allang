#compile_all expect.al

[Arr] :: 5*i32{.. 0} =[]

Arr..

Arr.1..

Arr..1

Arr..5

Arr.1..1

Slice :: Arr..

I :: 1

J :: 3

Arr * I ! ret 1

Arr * I.. ! ret 1

Arr * I..3 ! ret 1

Arr * I..J ! ret 1

Arr * 1..I ! ret 1

ret 0

