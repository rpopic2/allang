## moving numeric literals to scratch register

```
mov/literal_to_scratch ::= literal/numeric[, %]*
literal/numeric ::= [typename ][-]0-9+[.0-9+] | literal/char
literal/char ::= "'.'" | "'space'" | "'newline'" | "'nul'" | "'\n'" | "'\r'" | "'c-.'"
```
### examples

0   // moves 0 to a register.
    // moves to a return register if this is the last line

0.1f, 'c'       // moves a floating point number and a character
                // to two distinct registers
'newline' 'c-x' // newline and control-x character

### assembles to

mov instruction.
may be assembled to xor self, self.



## arithmetics to scratch register

```
arithmetic/literal ::= literal/numeric op/binary literal/numeric[, %]
op/binary ::= '+' | '-' | '*' | '/' | 'lsl' | 'ror' | 'asr' | 'addc' | 'adds' | '?' | 'mul' | 'div'
```

### examples

1 + 3           // adds 1 and 3 into a register
1 - 3, 1 * 3    // subtracts to a register, mutiplies to another register
1 / 4           // result is in integer like c
1.0 / 0.3       // result is in float

1 lsl 3         // shifts are available
2 ror 4
3 asr 5
2 addc 4        // add with carry and set flags are available
3 adds 5

3 mul 8         // disables constant multiplication optimizations
4 div 2

### assembles to

arithetic operations. division and multiplication with literals will be optimised
unless 'mul' and 'div' operators are used.


## named register declaration

```
decl/nreg ::=
    | id/nreg :: expr/line
    | id/nreg ::\n\texpr/block

id/nreg ::=
    | A-Z[a-zA-Z0-9._/<sp>]*
```

0 is special type of register.

### compiles to

named register has semantic equal to callee-saved registers.
named registers can be converted to stack variables if run out of callee-saved registers specified in the abi, however the semantic should be conserved.

the size of a type that named register is holding should be less or equal to the size taht register can hold.

### examples

I :: 1          // declare named register
                // zero is moved to register i

J ::
    i * 4           // a block (multiple lines of expressions) is allowed
    bytes alloc=>   // the result of last expression is assigned to the named register.

### assembles to

this declaration is only meaningful to the programmer.
expressions on the right hand side will be assmebled.



## data processing on named registers

```
arithmetic/nreg ::=
    | id/nreg((literal/numeric))
    | id/nreg((arithmetic/literal))
    | id/nreg((op/binary= literal/numeric))
```

I(4)            // moves 4 to register i
I(3 + 4)        // all data processing expressions are allowed
I :: "hi"       // you can also redeclare named register with same name

I(i + 4)
I(+= 4)         // can be abbriviated like this
I(LSL= 4)
I(-4)           // moves -4
I(-= 4)         // moves i - 4
// I(- 4)       // this is invalid syntax

I :: 4          // or just redefine it. redefinition of scratch register is allowed in only same scope
I :: "hi"       // type can be changed on redefinition

### assembles to

correspoinding arithmetics.



## moving named to scratch registers

```
mov/named_to_scratch ::=
    | id/nreg
    | id/nreg op/binary (id/nreg | lit/numeric)
    | lit/numeric op/binary id/nreg
```

I               // moves register I to scratch register
I + 4           // does not move register I. adds I + 4 into a register




## ternary conditional selects

```
op/ternary ::= expr/bool ? nreg : nreg
```
// these are quirks from arm architecture

I < 4 ? 3 : 5   // use ?: operator
print=>         // and print!

I < 4 ? J : P + 4   // error! no operators can be involved!
                    // nested ?: operator is also not allowed

## operator precedence

// there is no operator precedence.

1 + 4 * 2
print=>             // prints 10

1 + (4 * 2)
print=>             // prints 9

4 * 2 + 1
print=>             // prints 9


## loading addresses of aggregates

```
literal/aggregate
```

"Hello World"   // calculates the address of a string in text section to a register
<1, 2, 3, 4>    // an array literal
{ 1, "hi" }     // a struct literal


## moving to return registers

expressions at the last line of a block will be moved to return register instead of scratch registers.

### examples

```
foo: (=>int)
    3
```


## moving to parameter registers

expressions before function call are moved to parameter registers instead of scratch registers.

```
expr/param ::= 
    | mov/literal_to_scratch
    | mov/named_to_scratch
    | arithmetic/literal
    | arithmetic/nreg
    | literal/aggregate

mov/param ::=
    | expr/param fn_call
    | expr/param\nfn_call
    | id/fn expr/param=>
```

### examples

```
3, 4 add=>

3, 4
add=>

add 3, 4=>
```
