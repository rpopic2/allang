## function call
```
fn/call ::= id/fn=>

id/fn ::= [_]a-z[a-zA-Z0-9_]
```

### assmebles to

`call`, `bl`


## function definition
```
fn/def ::= id/fn: ([fn/params ]=>[fn/returns])\n\tblock
```

### assembles to

this information is only meaningful to the programmer.


## return

```
return ::= "ret"
```

### assembles to

it jumps to return block, not actually compiling to `ret` instructions.



## branches and named registers

Sum ::
    1, 2        // moved to parameter registers. parameter comes before functions.
    add=>       // returned value save to register Sum

i :: 0
i print=>
i ret           // named registers survive after a function call



## conditional branches

```
branch/cond ::= expr/bool ->\n\tblock

expr/bool ::=
    | regable op/bool regable
    | regable ("is" | "isnt") "pointing"
    | op/bool

op/cmp ::= "is" | "isnt" | ">" | "<" | "<=" | ">="
```

// >, >=, <, <=, is, isnt | are available.

i is 0 ->
    "zero" print=>  // printed only when I is zero.

i >= 3 ->
    "greater or equal" print=>

// equal, carry, minus, overflow | are available.
U adds 1 is overflow ->
    "overflowed" print=>    // printed only when addition overflows

U subs 1 isnt negative ->
// note only flag-setting instructions are available


## conditional branches with boolean operations

```
op/bool ::=
    | expr/bool[\n | <sp>]("and" | "or") expr/bool
```

I is 0 or I is 3 ->
    ...

I is        // write cleaner
0 or 3 ->
    ...
I is
3 or <= 2 ->

I is 3
and J is 4 ->

