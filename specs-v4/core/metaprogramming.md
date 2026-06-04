## inline macro

// inline macros are type checked

@inline add: (i32 i, i32 j)
    +

1, 2 @add   // this expands to 1 + 2

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

