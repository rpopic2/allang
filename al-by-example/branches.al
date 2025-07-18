// # labels

foo: // this is a label. you can reference the next line of code with this.
    "foo" print=>

// #branches (jump)
->foo // branches to to the label foo. note that after printing foo, it will branch to it again, making this an infinite loop.


// # routines

bar: ()
    "bar" print=>
    ret // explicit return

bar: ()
    "bar" print=>
    // implicit return, because next line is not indented less or end of file.

// # branch with link (call)

bar=> //calls it, and comes back and executes next line.
"done!" print=>

// ->bar // compile err! routines only can be branched with link!

// #functions

baz: (i32 a, i32 b =>i32)
    a + b ret

1, 2 baz=> print=> // prints 3
// 1 baz=> print=> // compile err! too few arguments
// 1, 2 baz=> // compile err! function result unused!

baz2: (=>i32 _) // now you can ignore result
    1 ret


// #scope rules

x :: 1
dummy:      // this label is not routine or subroutine, we go into this scope
    x(+2)
x print=> // 3

dummy2: ()  // this is a routine, so not executed unless branched to. this code is actually moved after the main procedure.
    x(+3)
x print=> // 3
dummy2=> print=> // 6

:   // an anonymous label
    x :: 1
// x print=> // canot access x

locals:
    y :: 10
// y is not available here

shadow:
    // x :: "hello" // you cannot shadow regs defined outer scope!
x :: "hello" // ok.

// #loops

i :: 0
loop:
    i(+1) print=>
    ->loop

// # breaks

i :: 0
loop2:
    i(+1) print=>
    i > 10 <- // breaks if i is greater than 10
    ->loop
"end!" print=>
// more type of loops on macro.al


// #nested labels

foo:
    bar: (=>i32)
        100
foo.bar=> print=>

foo.bar: (=>i32)
        100
foo.bar=> print=>


foo: (=>i32)
    200
    bar: (=>i32)
        100
foo=> print=> // 200
foo.bar=> print=> // 100

// foo: (=>i32)
//     bar: (=>i32)
//         100
//     200 //invalid. put it before other nested labels.


// # local variable semantics

foo:
    I :: 123
    bar:
        J :: 456
        K :: super.I + J
        // all variables resolevs to local ones by default
        Super.*I(+ 2)   // need to put mut sign(*) to change the value

// foo.bar-> // jumping to it would make the program in unpredictable state.
// if foo didn't have any code to run, this would compile.

foo:
    I :: 123
    baz: (=>)
        super.I // ? can access to it?
    baz=>

foo:
    Pointer :: std.allocate 10 bytes =>

    bar:
        2 =*[Pointer, 1 auto]

    std.free Pointer =>
    bar->   // dangling!

foo:
    Pointer :: std.allocate 10 bytes =>

    bar: (addr byte Pointer* =>)
        2 =*[Pointer, 1 byte]

    Pointer* bar=>
    std.free Pointer =>
    Pointer* bar=>   // compiler err!

// #subroutines

x :: 1

sub: subrt
    x(+1)
    // print=> subroutines cannot call functions

// x is 1 here, the code above does not get executed

subrt=>
// now x becomes 2

subrt=>
// now x becomes 3


// lhs, rhs omission in functions

add: (i32, i32  =>i32) // note that both are unnamed registers
    + // this is a valid code.

// parameters and registers

add: (i32 a, i32 b =>i32) // both are named scratch registers
    "hi" print=>
    // a + b ret // registers invalidated after call
    // also compilation err, this fn does not return i32

baz: (i32 a, i32 b =>i32) // both are named registers
    a, b :: ^ // copy them to named ones. shadows it.
    // :: or as shorthand
    "hi" print=>
    a + b

baz: (i32 a, i32 b =>i32)
    ::
    "hi" print=>
    a + b

baz: (i32 a, i32 b =>i32)
    a, b :: =[], =[] // shadow them.
    // =[] // or as shorthand
    "hi" print=>
    [a] + [b] ret // registers invalidated after call

baz: (i32 a, i32 b =>i32)
    =[]
    "hi" print=>
    [a] + [b] ret // registers invalidated after call


// #address of label, and function pointers

foo: (=>i32)
    40

foo // loads address of label to scratch register.

bar :: foo // bar holds addr of foo
bar=> // you can branch to it.

#typeof bar is addr (=>i32) @expect

// # main routine

// all file scoped declaration is inside main routine.
foo: (=>i32)
    40

main.foo=> print=> // 40

// to get args, put this at the start of the routine.
argc, argv :: ^ // moves argc and argv to a named register.
[argc], [argv] :: =[], =[]  // do this to store it on the stack.
// it has type of i32 argc, addr addr c8 argv

// or use macro.
args :: @get_args
// now args is slice of str.
// type of: slice str
args.len is 0 -> // do something
#at args[1], #str.equals "hello" -> // do another stuffs

args[1] @at, "hello" @str.equals -> // do sth

// # >> and <<

i is 10 -> "hi" print=> >>
"never print this" print=>
<<

i is 10 -> "hi" print=>>
"never print this" print=>
<<

