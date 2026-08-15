## metaprogramming with inline macro

`@inline` macro is a powerful tool for metaprogramming. it can replace a macro with code, types, values, functions.
however, relying on macro can make code bloat and may degrade compiling speed, so use it sparingly.

## inline macro

// inline macros are type checked

@inline add: I comptime_int, J comptime_int => comptime_int
    ret I + J

add 1, 2 @ // this expands to 3


## producing codes with inline macro

@inline add: I comptime_int, J comptime_int => code comptime_int
    I + J

@inline max: I comptime_int, J comptime_int => code comptime_int
    I >= J ? I : J

max 1, 2 @ // this expands to '1 >= 2 ? 1 : 2'. note that it may be folded to '2' when assembled.


## producing types with inline macro

```
@inline vector: I type => type
    @Typename: @"vector." .+ I .name @ @
    Typename @:
        struct {
            Begin addr I,
            End iter I,
            Capacity iter I,
        }
        add: &Self addr @Self, Value I =>
            [Self.End]++ := Value

vector i32 @ // instantiates type vector.i32

Vec :: vector.i32{.. 0} // init to zero just for example (this would segfault)
Vec .add 3 =>
```
side note: maybe we don't need to instantiate per type but per size of a type


## constant values with macro

Pi: 3.1415926535 // compile-time constant. replaces all occurances. it is also equal to defining new literals.
// compile-time constants' name start with a capital letter and be at least two letters.

Pi .write_to &Out =>

note: if you want the constants to have runtime memory address, use:
[Pi]: 3.141592 // now it's inside memory
if you want it to be mutable, use:
[data.&Pi]: 3.14 // now modifieable. must be in data to be modifiable (text sections are read-only)


## enum

an enum is just a namespace. it does not allow members with duplicate value. it also do not allow members with different types. value of a member starts with zero and increments by 1 automatically, if value is not specified and if the underlying type is a int type.

```
suit:
    @enum: // underlying type is comptime int by default
        Clubs:
        Spades:
        Diamonds:
        Hearts: 3 // explicit number

    @inline is_clubs: Self suit => ?
        Self is suit.Clubs

card:
    struct {
        Suit suit{u2}, // you can set the underlying type here
        Number u4,
    }
```

the underlying type for enum can be something other than int, as members are just compile-time constants.

```
@enum{point}:
    Zero: {0, 0}
    One: {1, 1}
```

## declare directive
example:
```
#declare printf: Format slice u8?, Args c.variadic => Num_Chars_Printed i32
```
declare directive tells the compiler the symbol exists, and what signature it has. conflicting signatures are not allowed.

## private symbol and types

symbols and types prefixed with '_' are private and not imported. you can still import with `#import_all`, but don't forget to use with caution.

top-level statements are the implicit main function. this is not imported, but may be useful when writing tests for the module.

## import directive

example:
```
#import std.al
#import_all std.al
```

import directive imports all functions from the path provided.
* it will not import top-level statements (the implicit main)
* it will not import functions prefixed with `'_'`.
* same file will not be imported twice.

importing a symbol or type is equivalent to `#declare`ing it. it will not compile the body, but lets the compiler know the symbol or type exists.

import_all directive imports all symbols and types from the path, including those prefixed with `'_'`.

## no_import_all_self directive

#no_import_all_self
when compiling, self file is `#import_all`ed automatically by default. you can opt-out with this directive.

## allow_import_multiple_times

same file is not imported twice by default. you can opt-out with this directive.

## compile directive

example:
```
#compile std.al
#compile_all std.al
```

compile directive compiles a file at the path and imports symbols.
* it will not compile top-level statements(the implicit main).
* it will compile private functions (prefixed with '_'), but not import it.
* functions in the target file can reference a private function, but the current file can't.
note that allang does not have a notion of compilation unit. all .al files are compiled at once, unless you link with libraries.

compile_all is equivalant to compile directive, but will import all private symbols.

## directive paths

paths in directives are resolved relative to the directory of the file that contains the directive, not the compiler's current working directory.

paths must use forward slashes (`/`) as the separator, even on Windows. backslashes are not supported.

