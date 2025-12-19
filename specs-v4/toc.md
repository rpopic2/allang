# table of contents

allang is:
- simple
- low-level but productive
- consistant
language.

allang consists of three parts:

- assembler
- core
- safety features


### assembler

assembler compiles your code into machine codes.
it follows the arm architecture model as reference, but it should be capable of assembling virtually any other architecures.

it should be capable of compiling following things:

- data processing
- branches
- load and stores

data processing adds or multiplies data. this includes moving data from register to register.
branches makes program counter to jump.
load and stores, moves data from memory to registers and vice versa.


### core

core is the language itself, built-in and cannot be modified by the programmer.
these features are not very meaningful to the machine, but can be super useful, and even essential for the programmer.

- aggregates
- scopes
- metagramming

aggregates bind data together and form meanigful object for the programmer.
scopes lets object survive only for desired length, and mark start and end of a loop or a function.
metaprogramming is capable of generating code programatically.


### safety features

safety features helps programmer to check logic of their programmes without testing.
these safety features can be turned off.

- type system
- memory safety

allang is statically, strongly typed language. it checks types at the compile time.
though allang is not a garbage collected language, it provides various memory safety features,
such as tying lifetime of a resource to scope, reference-counted smart pointers.

