struct {
    A i64 B i64
}
s2:
    struct s3 {
        M main
    }

Dummy :: 0
Index :: 2

[S] :: main{.A 1 .B 2} =[]

// [Arr] :: 10*main{.0 main{.A 1, .B 2} .. 0} =[]
// Arrp :: Arr
// [Arrp, Index]
ret 0
