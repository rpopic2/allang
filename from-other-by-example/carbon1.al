int_result:
    enum union {
        Success i32,
        Failure str,
        Cancelled,
    }

parse_as_int: S str => int_result
#leaf
    mut R :: 0
    foreach C in S
        C is_digit !
            ret {Failure, "Invalid Character"}
        R := (10 * R) + C) - '0'
    ret {Success, R}

try_it: $S str =>
    Result :: parse_as_int $S =>
    Result
    . is {Success, &X} ? `Read Integer `X``n
    : . is {Failure, &Err} ? `Failure '`Err`'`n
    : . is {Cancelled} ? panic

    . print_to std:Out =>

try_it "12345" =>
try_it "12x45" =>
ret 0

