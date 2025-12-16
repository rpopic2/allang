# moving data

0   // moves 0 to a register.
    // moves to a return register if this is the last line

"Hello World"   // moves the address of a string in data section to a register

0.1f, 'c'       // moves a floating point number and a character
                // to two distinct registers
'newline' 'c-x' // newline and control-x character


# arithmetics

1 + 3           // adds 1 and 3 into a register
1 - 3, 1 * 3    // subtracts to a register, mutiplies to another register
1 / 4           // result is in integer like c
1.0 / 0.3       // result is in float

1 lsl 3         // shifts are available
2 ror 4
3 asr 5
2 addc 4        // add with carry and set flags are available
3 adds 5


# named registers

I :: 1          // declare named register
                // zero is moved to register i

J ::
    i * 4           // multiple lines of expressions are allowed
    bytes alloc=>   // the result of last expression is assigned

// named registers must start with capital letter

# data processing on named registers

I(4)            // moves 4 to register i
I(3 + 4)        // all data processing expressions are allowed
I :: "hi"       // you can also redeclare named register with same name

I(i + 4)
I(+= 4)         // can be abbriviated like this
I(LSL= 4)
I(-4)           // moves -4
I(-= 4)         // moves i - 4
// I(- 4)       // this is invalid syntax

# moving named registers

I               // moves register I to scratch register
I + 4           // does not move register I. adds I + 4 into a register


# conditional selects

// these are quirks from arm architecture

I < 4 ? "less than" : "equal or greater to" // use ?: operator
print=>

I < 4 ? J : P + 4   // error! no operators can be involved!
                    // nested ?: operator is also not allowed

# operator precedence

// there is no operator precedence.
// parenthesis are not allowed

1 + 4 * 2
print=>             // prints 10

4 * 2 + 1
print=>             // prints 9


