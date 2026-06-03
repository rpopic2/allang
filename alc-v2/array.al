#declare printf: Format addr u8 => Num_Printed i32!5
#compile_all expect.al

[Arr] :: 5*i32{.0 0 .1 1 .2 2 .3 3 .4 4 .5 5 .6 6} =[]
[Arr.0]
[Arr.1]
[Arr.2]
[Arr.3]
[Arr.4]

Arr.0
Arr.1
Arr.2
Arr.3
Arr.4

ret 0


