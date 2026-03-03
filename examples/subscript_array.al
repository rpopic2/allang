// Arr[I *].Memb

struct {
    X i32, Y i32
}

Arr :: 10*Self =[]

Y_Acc ::
    This := 0
    for I in 0 .. 109
        This :+ [Arr*I.Y]

// subscript syntax shoud:
// have no spaces
// maybe use aseterics?
// not verbose
// distinct from dot syntax
// sp + arr_off + (idx * size) + member

// remaining punctuations:
// !%^\|;
