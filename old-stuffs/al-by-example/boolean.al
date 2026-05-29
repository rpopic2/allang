// #literals

true        // you get the literals for boolean
false

b :: true
b is true   // do boolean with is operator

i :: 0

i is 0
i is 4
i isnt 10   // use isnt for is not
i < 0
i > 0
i <= 0
i >= 0

i isnt 10 and i isnt 9  // use and to chain them
i is 10 or 9   // use or to match two values

c >= 'a' and c <= 'a', or c >= 'A' and c <= 'Z' // comma before 'or' is mandatory. to split these up. you don't need to put braces around.
// note that there is no operator precedence in al!

// you can even put these in mutliple lines
c >= 'a' and c <= 'z',
or c >= 'A' and c < 'Z' ->
    "is alphabet!" print=>

// you can even do lhs omission!
c
>= 'a' and <= 'z',
or >= 'A' and <= 'Z' ->
    "is alphabet!" print=>

// we even have a macro for this
c
'a'..'z' @in_range,
or 'A'..'Z' @in_range

i is 9 ->   // no need to type if.
    "is 9!" print=>

// you can do switch with this
i
is 0 -> "is zero!" print=>>
is 3 -> "is three!" print=>>
isnt 1 -> "not 0 or 3 or 1!" print=>>
<<

i
is 0 -> "is zero!",
is 3 -> "is three!",
isnt 1 -> "not 0 or 3 or 1!",
: -> "other value"
print=> // it is required to be exhausive if you want to use lhs omission here

// this is if..else
i
is 3 -> "three" print=>,
is 4 -> "hi!" print=>,

// this is two if statements
i is 3 -> "three" print=> // notice that comma at the end is gone!
i is 4 -> "hi!" print=> // might be clearer to put a new line before this

// you cannot do lhs omission because after taking branch 'i is 3', the scratch registers are compromised!
// i
// is 3 -> "three" print=> //also generates warning that i is 3 can be written in one line
// is 4 -> "hi!" print=> // so makes a compile error! lhs missing for is 4!

// # ternary operator
i is 3 ? 10 : 40    // ok
// same, but you cannot next, or put expression inside ternary operator.

