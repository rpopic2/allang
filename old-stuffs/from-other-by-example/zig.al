Input :: "123 67 89,99"
List :: std:list u32 .new =>
~ List .free =>

&Num :: str.iter u8?{Input}
loop:
    loop tokenize: &Num
        [Num.End++] ! break->
        is ' ' or ',' break->
        tokenize->
    [Num] ! break->

    N :: try u32.parse Num

    try List .push N

    It.Begin := It.End

    loop->

&Num :: str.iter u8?{Input}
loop:
    Token :: str?; loop: &It
        [It++] ! -> Token := .Error break->
        is ' ' or ',' break->
        tokenize->
    str ! break->

    N :: try u32.parse Num

    try List .push N

    It.Begin := It.End

    loop->

