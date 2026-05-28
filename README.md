# allang: a programming language for modern hardware
by rpopic2

```
- low level as assembly, but simple as python
- explicit error handling, but not verbose
- write optimized code without optimizers, and compile fast
```


### why allang?
- interoperates with c.
- all variables are immutable outside declaration scope
- when it comes to performance tuning, you have to look at assembly anyway, and just hope that the compiler will generate the code you want.
- some languages like c++ and rust are so complex that learning how the actual computers work is simpler and beneficial
- we only target x86_64, aarch64, linux, macos and windows. which makes abstracting the real hardware much easier. (not targeting some ancient systems like 10-bit machines)
- known footguns are allowed but needs to be explicitly requested. (use unchecked to remove bounds checking, undefined to use uninitialized value)
- c compiler model and the compilation pipeline is based on the good ol' days' hardwares and constraints. we need a language that respects modern hardwares
- it is easy to write a simple program, like python. easily spin up a shell script.
- ergonomic design. type less, type easy.



hello.al
```
"Hello World!"n .print_to std.Out =>
```

format.al
```
Number :: 1234
`This prints `Number` to the `std.Out .name @`!`n
.print_to std.Out =>
// result: "This prints 1234 to the std.Out!"

```

error.al
```
get_third_elem: Slice slice i32 => option i32
    [Slice. 2] ! eret  // zero-indexed. ! operator checks bound. eret statement returns error.
```

vectorization.al
```
Array :: <1, 2, 3, 4, 5, 6, 7, 8, 9>

Sum :: 0 sum [Array 0..]
Sum2 :: 0 sum [Array 0..4]
```
