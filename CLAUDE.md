## Build & Run

Run from `alc-v2/`.

The emit backend files for your architecture and OS must be passed explicitly:

```bash
# Build the compiler (Linux x86_64)
./build.sh asm-x86_64.c asm-linux.c

# Compile and run an allang source file:
# ./run-asm.sh <asm-arch.c> <asm-os.c> <source.al>
./run-asm.sh asm-x86_64.c asm-linux.c hello.al
./run-asm.sh asm-aarch64.c asm-macos.c hello.al
```

## Done When

- `test-all.sh` passes.
- aarch64 emits assembly without error (no need to run it when you are in cloud conatiner).
- `ll.c` backend needs no testing.
- `examples/` contains ill-formed code; only `tests/` are valid programs.

## Coding Conventions

- Do NOT add comments. Only add a comment when it is strictly necessary to explain something the code itself cannot convey, and keep it minimal.- Make pointers const whenever possible.
- Always put `return` on its own line; never `if (cond) return x;` on one line.

## Architecture

The compiler is a single-pass compiler entirely in `alc-v2/`:

- **`main.c`** — Tokenizer (`tok()`), parser (`parse_block()`), type checker, and driver.
- **`typesys.h`** — Type system: fundamental types, composite types, declarators, member layout.
- **`emit.h`** — Abstract code generation interface.
- **`asm-x86_64.c`** / **`asm-aarch64.c`** — Architecture-specific implementations of `emit.h` (ASM text pipeline).
- **`asm-linux.c`** / **`asm-macos.c`** / **`asm-windows.c`** — OS-specific assembly preamble/sections.
- **`exe-x86_64.c`** / **`exe-aarch64.c`** — Architecture-specific instruction encoders (binary pipeline).
- **`exe-elf.c`** / **`exe-elf-x86_64.c`** / **`exe-elf-aarch64.c`** / **`exe-macho.c`** / **`exe-pe-x86_64.c`** — Binary executable format backends.
- **`ll.c`** — LLVM IR output backend.

**Data flow**: `.al` source → tokenizer → parser → type checker → `emit_*()` calls → platform emitter → `.s` assembly → clang → executable.

## Language Notes

allang uses indentation-sensitive scoping. Key syntax elements:
- `::` — variable declaration (all variables are immutable outside declaration scope)
- `=>` — function call (postfix: `arg .fn_name =>`)
- `[S]` — memory load (load from S)
- `X =[S]` — memory store (store X to S)
- `!` — bounds check operator
- `->` — branch/jump
- `@` — macros, `#` — directives
- `=` — assignment operator. note that this operator assigns from left to right.
See `specs-v4/` for the full language spec.
