# table of contents

allang is:
- simple
- low-level but productive
- consistent
language.

allang consists of three parts:

- assembler
- core
- safety features


### assembler

assembler compiles your code into a machine code, assembly code depending on your backend.
it follows the arm architecture model as reference, but it should be capable of assembling virtually any other architectures.

it should be capable of compiling following things:

- data processing
- branches
- load and stores

data processing adds or multiplies data. this includes moving data from register to register.
branches makes program counter to jump.
load and stores, moves data from memory to registers and vice versa.

the output is generated whether or not there was a compiler error, but the resulting assembly will have undefined behavior.

### abi considerations

the assembler will try its best to meet abi requirements, however, it is not guaranteed to do so. in order to strictly conform to the abi, use export keyword on your struct or function.

* a 'register' in allang has a maximum size of 16 bytes, where actual machines has max size of 8 bytes. registers greater than size of 8 will be split into two registers, while msvc x64 abi does not allow this for function calls.
* allang is capable of returning multiple registers, while most x86 calling conventions do not allow it.

### core

core is the language itself, built-in and cannot be modified by the programmer.
these features are not very meaningful to the machine, but can be super useful, and even essential for the programmer.

- aggregates
- scopes
- metagramming

aggregates bind data together and form meaningful objects for the programmer.
scopes lets object survive only for desired length, and mark start and end of a loop or a function.
metaprogramming is capable of generating code programmatically.


### safety features

safety features help programmer to check logic of their programs without testing.
these safety features can be turned off.

- type system
- memory safety

allang is a statically, strongly typed language. it checks types at the compile time.
though allang is not a garbage-collected language, it provides various memory safety features,
such as tying lifetime of a resource to scope, reference-counted smart pointers.

