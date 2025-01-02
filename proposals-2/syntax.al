# hello world

main:
    "Hello World!" ->.print

main:
    "Hello World!" =>.print

    // a token that ends with colon is an address
    // load and stores

main:
    i: i32      // creates a i32 object on the stack
    [i]         // loads i onto a scratch register
    +1          // increment 1
    =[i]        // store back to object on the stack

# registers
    // a token that starts with $ sign is a named register

    i: i32
    [i] >$r     // load i onto register r. operator > is equal to move instruction.
    $r + 1 >$r  // add 1 to register r
    $r =[i]     // store back to register r
    // this code is identical to the code above, except the register is explicit.

# arithmetics
    1 + 3 - 5 * [a] // arithmetic can be done in such a manner.
                    // [a] means to load a from memory.
                    // other way of doing arithmetics:
    [i], [a]        // load i, a from memory, but to different register.
    +   =>.tostring =>.print               // adds it, prints it.
    [i] + [a] =>.tostring =>.print          // equal thing.

# branches
main:
    ->foo           // do branches with -> operator.
    "don't print this" =>.print
foo:
    "print this!" =>.print

# conditional branches
    [a]
    ? 3 :: eq =>foo     // operator ? does cmp.
    ? 2 :: lt =>bar
    =>baz

    # equivalant in c:
    # if (a == 3)
    #   foo();
    # if (a < 2)
    #   bar();
    # baz();

    [a]
    ? 3 :: eq =>foo->fin
    ? 2 :: lt =>bar->fin
    =>baz
    fin: =>foobar

    [a] ?
        3 =>foo,
        2 :: lt =>bar,
        =>baz
    =>foobar

    # equivalant in c:
    #   if (a == 3) {
    #       foo();
    #   } else if (a < 2) {
    #       bar();
    #   } else {
    #       baz();
    #   }
    #   foobar();

    [a]
        ? 3 :: eq   // :: is an anonymus lable.
            =>foo   // eq followed by :: means that this label is an condition-
            =>bar   // al label. if the condition is not met, it will jump to
        ? 2 :: lt   // the next same 
            =>baz
    =>foobar

    [a]
    ? 3
    :: eq
        =>foo
    :: lt
        =>bar
    ::

# loops
    $i: 0
    loop: $i ? 10 lt
        ~$i + 1 >$i
        $i =>.tostring =>.print
        ->loop


    $i: 10
    loop:
        "hi" =>print
        $i - 1
        ->ne loop

    macro:
        "hi" =>print
    <macro> 10

# addr (pointers)
    i: i32
    $p: i   // holds address of i
    $j: [i] // holds value of i
    [i] >$j

    ptr: addr i32
    i =[ptr]    // ptr holds address of i32
    k: i32
    [i] =[k]    // k holds value of i32

# structs
    coord (x: i32, y: i32)

    start_pos: <coord>
    3 =[coord.x], 4 =[coord.y]

# arrays
    arr: i32 10
    $i: 10
    loop:
        $i =[arr, $i]
        $i: $i - 1
        ->ne loop
