fn main:
    // subroutines
    3, 7 -> sum -> "%\n" -> print

    // looping
    0
    i32 i
    loop:
        i -> "hi %\n" -> print
        i, 1 -> + -> i
        < 10 ? loop
        // ? is a conditional jump operator

    // i
    // inc  -> err! address expected.
    // &i   -> err! cannot take address of register
    let a; 0 => a
    & -> inc
    print

    // branching

    0 -> ret

fn sum:
    i32 a, b
    +
    ret

.inline
fn inc:
    ptr i
    + [i], 1
    =>[i]
    ret

