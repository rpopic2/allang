// this document explains detailed sematic of registers in al
// not intened to be introductory
// it requires some understanding of registers to read

// #simple mov

0

// the most simple program that you can write in al.
// this program returns 0.
// to be more specific, it moves 0 to the return value register
// it only moves to return register if it's the last line of a file or a block

// # unnamed registers

0       // mov 0 to a scratch register
1, 2, 3 // mov 1, 2, 3 to three distinct scratch register

i       // mov i to a scratch register

1       // if this is the last code in current scope, it moves to return value register, not to scratch register.


// # or lhs/rhs omission (register fallthrough)
1 + 2, 2 + 3   // add 1 + 2 to a scratch register, and 2 + 3 to another
+               // add two registers above to a scratch register

1 + 3       // add 1 + 3 to a scratch register
+ 4         // and add 4 and the result above to a scratch register

// # named registers

i :: 0      // move 0 to named register i
j :: 1 + 4  // add 1 + 4 to named register j
// p :: 1, 2    // not allowed to mov two values!
p :: { 1, 2 }    // do this instead. two i32 will be packed into one 64bit reg

// named registers are callee-saved registers that can survive after function calls.
// it moves 0 to a named register i.
// also, what j gets assigned to is the last result of the rhs.


// i               // i is not declared!
i :: 0
i = 10         // moves 10 to i.
// i("hello!")    // compile error! named registers are type checked.
// i(4.0)         // floats are not automatically casted to ints
i :: "hello" // shadows i!

// declare multiple of them 
i, j, k :: 1, 2, 3


i : 1 + 4    // add 1 and 4 to register i

1 + 4       // do 1 + 4 and move to scratch register
i         // and move the result above to i.. which is probably what you don't want to do! generates warning.

i
+ 4     // warning! it can be written in one line!


// multiple line assignment

i :: 0

a ::
    i is 0 -> "zero" >>
    j isnt 0 -> "non-zero" >>
    <<

a print=> // zero


// # stack objects

[obj] :: i32 // you can just declare it before using
[i] :: 123 =[]    // mov 123 to a scratch register, store it on stack. name that object i. size of a int literal is 32-bit by default
[i] :: 123 =[i] // same code, more verbose.

// note that i is not a register, but a offset from a stack pointer.

i   // adds stack pointer and offset, which produces addres to the stack obj.
    // in other words, i is a pointer to the stack pointer.

[i] // load the value of stack object i

14 =[i] // mov 14 to a scratch register, storing it to object i

// i = 14  // is is type of stack i32. cannot assign 14 to it.
[j] :: 0 =[]
// i(j)        // illeagal syntax: stack i32 is not assignable. it's not a register!! it's simmilar to doing something like 4(j).

#offsetof i // if you want just an offset of it

// # arithmetic operators

// 1 + 2 * 3 + 4   // warning! there is no operator precedence!
// also, chaining the expression with different operation generates warning. you need to split it up

1 + 2, 3 + 4    // do 1 + 2 to a register, and do 3 + 4 to another register
*               // multiply them

1 + 2       // you can
* 3
+ 4

1 + 2

// * 3 // compile error if you put newline between! lhs is missing!

1 + 2
print=>
// * 3 // compile error! scratch registers do not survive after function calls

5 / 2           // division. but won't perform optimizations like 5 >> 1

i, 2 @div
// if you are dividing with constant, it's better use to macro. now it'll use 5 >> 1 optimization.
i, 2 @mul   // same with multiplication

i orr j   // do bitwise artimatics
i and j
i xor p


// # move to symbol (%)

foo=> % // move the result of foo to next matching %..
"result was: ", % printf=>    // which is the argument 1 register!
// % printf=>  // compile error! it needs to be in pair!

// # named register, named with symbol ($)

$ :: "d" // i don't care about the name of it
$ scanf=>
$ printf=> // it survives across fn calls
$$ :: "f" // use more $ signs if you need to assign new

// # using $ symbol for clarity

foo: (=>addr element~)
    #sizeof element std.heap.alloc=> $ //$ is now the first return register
    self =[$]
    $ ret


// register alias (&)
d :: 123
a :: &d // a is another name for d. does not copy it.

s :: { 123, 456 }
s is s { 123, &b } -> b print=>   // new name b in this scope

