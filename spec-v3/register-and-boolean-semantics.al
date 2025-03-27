// register semantics

i :: 10 // a named register i. mov 10 to callee-saved registers
i       // mov w8, i
i foo=> // mov w0, i; bl foo
i <-    // w0
i ret   // w0
i       // if at the last line of the block, then...
        //      if it was in routine, to w0
        //      else if it was in :: block, to named register
        //      else .. cannot happen

j :: i  // mov i into disticnt callee-saved registers

i is j ->   // no move incurred. just cmps two registers.

i, j, k, l // use w8~w11
j, j, k, l foo=>    // use w0, w4

// compare and branch

t :: true
f :: false

t is true->    // tbnz
t is false->   // tbz

t is
true->
false->

n :: null
n is null->     // tbnz
n isnt null->    // tbz


i is zero-> // tbz
i isnt zero->

i is 1
i is !2

i is > 3 -> // cmp i, 3 gt
i > 3 ->    // is elidable
i >= 3 ->
i <= 3 ->

i is
> 3
>= 3
<= 3

// boolean operations

i is zero and j is 2 ->
i < 3 or j >= 5 ->

i is zero and j is 2 and p <= 2 -> //

i is zero and j is 2    // can span multiple lines
and p <= 2 ->

p is 5 or, p is 7 ->    // comma is mandatory after or

// but or is not mandatory (maybe we'll remove or from lang)
i >= 2 and p <= 5, p <= 2 and j is 4 -> // put comma
i >= 2 and p <= 5 and j <= 2, j is 4 -> // put comma

// lhs, rhs ommision

add (i32 a, i32 b ->i32)
    +   // lhs and rhs is automatically w0, w1, calculated onto w0

i :: 0
is 5    // cmps with register i

a, b, c, d :: { 1, 2, 3, 4 }
a + b, c + d    // a + b to w8, c + d to w9
*               // mul w8, w9. it it is the last line of the scope, w0

// inbetween fn calls
foo: (=>i32)
    3

// foo=>, foo=>    // illeagal! it stores to tmp register, so it is not garanteed that the values will survive between calls!

f :: foo=>
b :: foo=>
f, b

// or save it on the stack..
f :: foo=> =[]
b :: foo=> =[]
[f], [b]

foo=>, 3, 4 bar=>     // ok
5, foo=> bar=>        // illagal! 5 will not survive and replaced to 4

3       // after fn call, 3 is not accesible. compiler might complain
foo=>

// % registers
foo=> %     // moves % to the next pair of %...
1, % bar=>  // which turns out to be w1!

// foo=> %     // % does not survive accross fn calls!
// bar=>
// 1, % bar=>  // now both % are unmatched!

// foo=> %
// 1, %, % bar=>  // % must be used as pairs!

foo=> %, %1     // instead, make multiple pairs of %!
1, %, %1 bar=>  // use it with arbitary numbers  
