
@loop
    scan.i32=> =[a]; scan.c8=> =[op]; scan.i32=> =[b]

    [a, b, op]
    op is '+' -> + >>
    op is '-' -> - >>
    op is '*' -> * >>
    op is '/' -> / >>
    : "unknown operator" print=> continue->
    <<
    print=>


@loop
    scan.line=> ' ' str.split=>
    [0] @at i32.from.str=> $a; op :: [1] @at; [2] @at i32.from.str=> $b;

    $a, $b
    op switch
    is '+' -> + >>
    is '-' -> - >>
    is '*' -> * >>
    is '/' -> / >>
    : "unknown operator" print=> continue->
    <<

@loop
    a :: scan.token=> @try; i32.from=>
    op :: scan.token=> @try; [0] @at
    b :: scan.token=> @try; i32.from=>

    $a, $b
    op switch
    is '+' -> + >>
    is '-' -> - >>
    is '*' -> * >>
    is '/' -> / >>
    : "unknown operator" print=> continue->
    <<

@loop
    a :: scan.i32=> @try
    op :: scan.c8=> @try
    b :: scan.i32=> @try

    a, b
    op is '+' -> + >>
    op is '-' -> - >>
    op is '*' -> * >>
    op is '/' -> / >>
    : "unknown operator" print=> continue->
    <<
    print=>

@loop
    a :: scan.i32=> @try =[]
    op :: scan.c8=> @try =[]
    b :: scan.i32=> @try =[]

    [a, b]; [op] &op
    op is '+' -> + >>
    op is '-' -> - >>
    op is '*' -> * >>
    op is '/' -> / >>
    : "unknown operator" print=> continue->
    <<
    print=>

