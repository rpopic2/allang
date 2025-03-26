// https://gobyexample.com/if-else

#alias std.io.print

// modulo it is actually a macro..! it does whatever optimizations possible
// calculating modulo fast is not done simple
7, 2 @%.eq 0 -> // it would be 7 % 2 == 0 in c.
    "7 is even" print=>>    // =>> it jumps to next << after returning
->
    "7 is odd" print=>>
<<

8, 4 @%.eq ->
    "8 is divisible by 4" print=>

(8, 2 @%.eq 0) -> (7, 2 @%.eq 0) -> // chains it.
    "either 8 or 7 are even" print=>

num :: 9
num < 0 ->
    "is negative" print=>>
num < 10 ->
    "has 1 digit" print=>>
->
    "has multiple digits" print=>>
<<

// but you could do it better!!! this is faster and more maintainable
num :: 9
num
    < 0 -> "is negative" >>
    < 10 -> "has 1 digit" >>
    -> "has multipl digits" >>
<<
print=>


7, 2 @%.eq 0 ->
    "7 is even"
->
    "7 is odd"
<<
print.ln=>

7 @/ 4 // we also have a macro for division


// https://doc.rust-lang.org/stable/rust-by-example/flow_control/if_else.html

// 1. it's a assembly like way of doing it. not recommened unless you are hacking
n :: 5
// ? is a cmp (compare) operator.
n ? 0 < ->
    n" is negative"
n ? 0 > ->
    n" is positive"
->
    n" is zero"
print.nl=>  // it's a print with no linebreak! not a typo

// 2. more readable way
n < 0 ->
    //...

// 3. better way
n   // just pin it on w8 register
? 0 < ->
? 0 > ->
->

big_n ::
    n < 10 -> n > -10 ->
        ", and is a small number, increase ten-fold" print=>
        10 * n <- // this operator breaks and escapes current scope
    ->
        ", and is a big number, halve the number" print=>
        n / 2 <-

n" -> "big_n println=>
