## Build & Run

Run from `alc-v2/`.

The emit backend files for your architecture and OS must be passed explicitly:

```bash
# Build the compiler (Linux x86_64)
./build.sh emit-x86_64.c emit-linux.c

# Compile and run an allang source file:
# ./run.sh <arch-emit.c> <os-emit.c> <source.al>
./run.sh emit-x86_64.c emit-linux.c hello.al
./run.sh emit-aarch64.c emit-macos.c hello.al
```

## Architecture

The compiler is a single-pass compiler entirely in `alc-v2/`:

- **`main.c`** — Tokenizer (`tok()`), parser (`parse_block()`), type checker, and driver.
- **`typesys.h`** — Type system: fundamental types, composite types, declarators, member layout.
- **`emit.h`** — Abstract code generation interface.
- **`emit-x86_64.c`** / **`emit-aarch64.c`** — Architecture-specific implementations of `emit.h`.
- **`emit-linux.c`** / **`emit-macos.c`** / **`emit-windows.c`** — OS-specific assembly preamble/sections.
- **`emit-ll.c`** — LLVM IR output backend.

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

See `specs-v4/` for the full language spec.

## Code Conventions

- Use comments sparingly.
