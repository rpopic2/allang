
main:               // this is the entry point for your program.
                    // and add a colon which looks much like a lable!
    :: i (4 bytes)  // give memory a name and a size
                    // compiler will a alloc a mem addr for you on the stack.
                    // now, i is an alias for mem addr, which has 4 bytes
                    // this will not issue any instructions,
                    // but will allocate more space on the stack.
    // let's say i has address equal to sp

    0 (4 bytes)     // this will issue mov instruction: 0 to register 0.
                    // optionally specify width of literal.
    // or `0 (i32)`
    // mov r0, #0

    => i            // and this will issue str instruction: reg 0 to addr i.
    // str r0, [sp]

    // or you could write this in short:
    0 => ::i (4 bytes)
    // or even as this:
    0 ::> i

    [i]             // this will issue ldr instruction.
                    // i is an alias for where sp points to.
    // ldr r0, [sp]

    // let's do some arthimetics.
    3, [i]            // this will move 3 to reg 0, load i to reg 1.
    // mov r0, #3
    // ldr r1, [sp]
                    // you can load up to 8 values w/ comma.
    +               // this will issue an add instruction into r0!
    // add r0, r0, r1
                    // since + is an binary operator, it will use what's on r0 and r1.

    3, [i] +          // or you could write this as one liner.
    (3, [i]) +        // you could optionally add parenthesis to avoid confusion

    [i], 3 + => i   // ...and assign it to back i!

    [i]++ =>        // this will increment i by 1 and assing to it back
    [i] ++>         // there's even a shorthand for this!


    // copying values
    0 ::>  i (i32)
    :: j (i32)
    [i] => j

    // moving values

    [i] =0> j    //  move will leave 0 at where it's moved from,
                //  assigning the value moved to the destination.
    // ldr r0, [sp]
    // str r0, [sp, #4]
    // mov r0, 0
    // str r0, [sp]

    [i] => j    // which is basically equal to this.
    0 => i

    0
    <-        // this will return 0 to the caller and exit the main routine.
    // or you could write in one line:
    0 <-

    // how to reference values? let's talk about routines before that.

add <- i32: // note that this is a return value, not formal arguments
    => :: a (i32), b (i32) // these are the formal arguments
    + <-        // return sum of a and b.


main:
    3, 4
    -> add
    => ::sum (i32)
    // you call subroutine by using -> operator.
    // -> operator is called subroutine call operator.
    // it will branch to the subroutine's address.

increment:
    => :: p (ptr i32) // it recieves pointer to i32
    // str x0, [sp]
    :: $ref         // $ is a register sign. you can have named registers;

    $ref [p]    // load it up to the named reg,
    // ldr x9, [sp]

    [$ref]          // derefernce it, to the reg 0,
    // ldr x0, [x9]
    ++          // increment it,
    // add x0, x0, 1
    => $ref     // store it to where if points to,
    // str x0, [x9]
    <-         // and return.

main:
    1 ::> a
    a -> increment // note that we did not add [] operator. 
                    // this will mov address of a to register.
                    // note that name without [] will move into reg.
    // mov x0, sp
    // or: add x0, sp, #0x10
    <-

increment:
    [] ++>;
    /* you could write this in short.
    since argument is already in x0,
    [] just dereference it,
    ++> increment and assign
    since no operand is specified, it will use x0
    ; and return. ; means end of this block. this routine would get inlined.
    */


main:
    3 ::> a (i32)
    a
    -> increment

