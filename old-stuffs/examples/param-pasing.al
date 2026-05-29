struct point {
    X i32
    Y i64
}

foo: (P point =>)
    printf
        va.0 = P.X + P.Y
        format = "%ld"
        =>

foo: (P point =>)
    % = P.X + P.Y
    printf "%ld", % =>


foo: (P point =>)
    printf "%ld", P.X + P.Y =>


foo: (P addr point =>)
    %1 = [P.X] + [P.Y]
    printf "%ld", =>

foo: (P addr point =>)
    printf "%ld", [P.X] + [P.Y] =>

$[A]

$A

