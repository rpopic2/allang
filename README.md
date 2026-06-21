# allang: a programming language for modern hardware
by rpopic2

```
- low-level as assembly, simple as python
- explicit error handling, without the verbosity
- write optimized code without an optimizer, and compile fast
```


### why allang?
- interoperates with c.
- all variables are immutable outside their declaration scope.
- performance tuning means reading the assembly anyway and hoping the compiler emits what you want; allang lets you write it directly.
- languages like c++ and rust are so complex that learning how computers actually work is simpler, and pays off more.
- it targets only x86_64, aarch64, linux, macos and windows, which makes abstracting real hardware far easier. (no ancient systems like 10-bit machines.)
- footguns are allowed, but must be requested explicitly. (use `unchecked` to remove bounds checking, `undefined` to use an uninitialized value.)
- the c compilation model reflects the hardware and constraints of decades ago; we need a language that respects modern hardware.
- writing a simple program is easy, like python. spin up a shell script in seconds.
- ergonomic design: type less, type easy.


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
    [Slice. 2] ! eret  // zero-indexed. ! checks bounds. eret returns an error.
```

vectorization.al
```
Array :: <1, 2, 3, 4, 5, 6, 7, 8, 9>

Sum :: 0 sum [Array 0..]
Sum2 :: 0 sum [Array 0..4]
```
