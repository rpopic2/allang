// 2557

"Hello World!" =>print

// 1000
$a: i32, $b: i32 =>scan
+
=>tostring =>print

// 10998
$a: i32, $b: i32 =>scan
a * b =>tostring =>print

// 10869
$a: i32, $b: i32 =>scan
+ =>tostring =>println
- =>tostring =>println
* =>tostring =>println
/ =>tostring =>println
% =>tostring =>println

// 1008
$a: f32, $b: f32 =>scan
a / b =>tostring =>println

// 11382
$a: i32, $b: i32, $c: i32 =>scan
a + b + c =>tostring =>println

// 1330
main:
    $a: i32, $b: i32 =>scan
    a ? b match
    :: gt
        ">"
    :: le
        "<"
    ::
        "=="
    =>println

// 9498
main:
    $score: i32 =>scan
    score match
    :: ? 90 ge
        'A'
    :: ? 80 ge
        'B'
    :: ? 70 ge
        'C'
    :: ? 60 ge
        'D'
    ::
        'F'
    =>println

// 14681
main:
    $x: i32, $y: i32 =>scan
    switch
    :: x ? 0 gt :: y ? 0 gt
        '1'
    :: x ? 0 lt :: y ? 0 gt
        '2'
    :: x ? 0 lt :: y ? 0 lt
        '3'
    :: x ? 0 gt :: y ? 0 lt
        '4'
    ::
        panic
    =>println

// 2753
main:
    $year: i32 =>scan
    switch
    :: year % 4 eq :: year % 100 ne :: year % 400 eq
        '1'
    ::
        '0'
    =>println

// 2420
main:
    $n: i32, $m: i32 =>scan
    n - m; abs =>tostring =>println

// 2741
main:
    $n: i32 =>scan
    i: 1
    :: loop ~i +: 1
        i ? n gt ->break
        i =>tostring =>println

// 10872
main:
    $n: i32 =>scan
    
    i: 1
    acc: i
    :: loop
        i ? n ge ->break
        ~i +: 1
        acc *: i
    acc =>tostring =>println

// 10950
main:
    $t: i32 =>scan
    :: loop t ? 0 ne
        ~t -: 1
        $a: i32, $b: i32 =>scan
        + =>tostring =>println

// 10952
main:
    :: loop
        $a: i32, $b: i32 =>scan
        :: a ? 0 :: b ? 0
            ->break
        a + b =>tostring =>println
    break:

// 2739
main:
    $n: i32 =>scan
    n_str: n=>tostring
    i: 1
    :: loop
        i ? 9 gt break
        i_str: i =>tostring
        prod_str: n * i =>tostring
        "% * % = %", n_str, i_str, prod_str =>println

// 2438
main:
    $n: i32 =>scan
    i: 0
    :: loop i ? n lt; ~i +: 1
        j: i
        :: loop j ? i lt; ~j +: 1
            '*' =>print
        '\n' =>print

// 10951
main: loop
    err: ($a: i32, $b: i32 =>scan try)
    :: err
        ret
    [a], [b] + =>tostring =>println

// 10871
main:
    $n: i32, $x: i32 =>scan
    $list: std.list, i32, n =>std.list.new

    i: 0
    :: loop i < n ~i +: 1
        i * sizeof i32
        + [list.data] =>scan

    i: 0
    :: loop i < n ~i +: 1
        i32, list, i #std.list.get
        list




