branch: (Num int =>int)
    :: Num is 0
        printf "%d", Num=>
    ::
        Num = 2

    ret Num


branch: (int =>int)
    ret
        isnt 0 ->
            =
            printf "%d", Num =>
        else
            = 2

branch: (Num int =>int)
    Num ::
        ..Num isnt 0
            printf "%d", Num=>
            = ..Num
        else
            = 2
    Num

// asm
branch: ($Num int =>int)
    Num ::
        $Num is 0 skip->
            = $Num
            printf "%d", Num=>
            fin->
        skip:
            = 2
        fin:
    ret Num

// c
branch: (Num int =>int)
    Num isnt 0 ->
        printf "%d", Num=>
    else
        Num += 2
    ret Num


// prefix $ if want to keep temp reg
branch: ($Num int =>int)
    Num ::
        $Num isnt 0 ->
            = $Num
            printf "%d", Num=>
        -> else:
            = 2
        :
    ret Num

// asm if..elseif..else
branch: (Num int =>int)
    Num < 6 skip->
        print "hello"=>
        fin->

    skip:
    Num is zero skip2->
        print "%d", Num=>
        fin->

    skip2:
        Num = 2

    fin:
    ret Num

// al if..elseif..else
branch: (Num int =>int)
    Num > 5 ->
        print "hello"=>
    else Num is zero skip2->
        print "%d", Num=>
    else
        Num = 2
    :

    ret Num

// asm if if..else

branch: (Num int =>int)
    Num < 6 skip->
        print "hello"=>
        next->

    skip:
    Num is zero skip2->

    next:
        print "%d", Num =>
        fin->

    skip2:
        Num = 2
    fin:

    ret Num

// al if if..else
    Num > 5 ->
        print "hello"=>

    Num is nonzero ->
        print "%d", Num=>
    else
        Num = 2
    :

    ret Num
