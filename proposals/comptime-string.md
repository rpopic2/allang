# comptime string?

should we introduce a concept of comptime string?
it means a string literal can be provied as a function argument that expects any of the following types:

1. addr 12*u8

X :: "Hello World"

2. 12*u8

[Y] :: "Hello World" =[]

3. slice u8

Z :: slice u8{"Hello World"}
assert.that Z.Length is 12 @

4. raw_ptr !u8 (c string)

strcmp "Hello World", "Bye World" =>


### array to other types

Y2 :: Y // now Y2 is a type of addr 12*u8
Y3 :: Y.. // now y3 is a type of slice u8


# types of pointers

1. addr

equivalant to const *const in C.
it cannot be subscripted, unless it points to an array of known length
cannot change where it's pointing to.

2. slice

an addr and a length.
can be subscripted
cannot change where it's pointing to.

3. raw_ptr

equivalant to raw pointers in c.
can be subscripted
can change where it's pointing to.

