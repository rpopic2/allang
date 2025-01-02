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

fn main:
    a, b -> scan(i32, i32)
    [a], [b]; + => [sum]
    "%d", [sum] -> fmt -> print
    0 <-

fn print:
    :: str (char span)
    1, [str.ptr], [str.size], 4~>$16
    syscall
    ret


    [i]

    8 (i32) ::= size
    :: p (8 bytes)

struct span:
    span

literal         mov
ascii names     alias to memory address
=>              store to memory address operator
->              jump
~>              w
::>             move operator
:>              t
|>
$>              move to named register
_               preserve register

@>
!>
#>
%>
&>
*>
+>

[]              dereference memory address operator
+               add operator
-               sub operator


