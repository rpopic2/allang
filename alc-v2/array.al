struct {
    A i32 B i32
}

Dummy :: 0
Index :: 2

[Arr] :: 10*main{.0 main{.A 1, .B 2} .. 0} =[]
[Arr] :: 10*main<.0 main<.A 1, .B 2> .. 0>[] =[]
Arrp :: Arr

ret [Arrp, Index]
