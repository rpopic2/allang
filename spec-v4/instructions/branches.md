# branches

print=>         // function call (branch to to function and return)
ret             // returns from function

continue->      // branches to a label called continue
<-              // breaks from current block

# branches and named registers

Sum ::
    1, 2        // moved to parameter registers. parameter comes before functions.
    add=>       // returned value save to register Sum

I :: 0
I print=>
I ret           // named registers survive after a function call

# conditional branches

// >, >=, <, <=, is, isnt | are available.

I is 0 ->
    "zero" print=>  // printed only when I is zero.

I >= 3 ->
    "greater or equal" print=>

// equal, carry, minus, overflow | are available.
U adds 1 is overflow ->
    "overflowed" print=>    // printed only when addition overflows

U subs 1 isnt negative ->
// note only flag-setting instructions are available

# system calls

1 syscall>          // calls system call 1

# boolean operations

I is 0 or I is 3 ->
    ...

I is        // write cleaner
0 or 3 ->
    ...
I is
3 or <= 2 ->

I is 3
and J is 4 ->

