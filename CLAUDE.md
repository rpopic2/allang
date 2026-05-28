# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

All compiler work happens in `alc-v2/`. Run from that directory.

The emit backend files for your architecture and OS must be passed explicitly:

```bash
# Build the compiler (Linux x86_64)
./build.sh emit-x86_64.c emit-linux.c

# Build the compiler (macOS aarch64)
./build.sh emit-aarch64.c emit-macos.c

# Compile and run an allang source file:
# ./run.sh <arch-emit.c> <os-emit.c> <source.al>
./run.sh emit-x86_64.c emit-linux.c hello.al
./run.sh emit-aarch64.c emit-macos.c hello.al
```

`build.sh` compiles `main.c` plus the provided emit files with clang (C11, `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror`, UBSAN on non-Windows) into the `alc` binary. `run.sh` internally calls `build.sh` with `$1 $2`, then runs `alc` on `$3`, assembles the output `.s` with clang, and executes the result.

## Architecture

The compiler is a single-pass compiler entirely in `alc-v2/`:

- **`main.c`** — Tokenizer (`tok()`), parser (`parse_block()`), type checker, and driver. All parsing and semantic analysis lives here.
- **`typesys.h`** — Type system: fundamental types, composite types (structs/unions/arrays), declarators, member layout.
- **`emit.h`** — Abstract code generation interface (platform-agnostic operations: MOV, ADD, LDR, STR, branches, etc.)
- **`emit-x86_64.c`** / **`emit-aarch64.c`** — Architecture-specific implementations of `emit.h`.
- **`emit-linux.c`** / **`emit-macos.c`** / **`emit-windows.c`** — OS-specific assembly preamble/sections.
- **`emit-ll.c`** — LLVM IR output backend.

**Data flow**: `.al` source → tokenizer → parser → type checker → `emit_*()` calls → platform emitter → `.s` assembly → clang → executable.

Support headers (all `#include`-only, no separate `.c`):
- `allocator.h`, `str.h`, `arr.h`, `dyn.h`, `hashmap.h`, `mini_hashset.h`, `err.h`, `opt.h`, `buffer.h`

## Language Notes

allang uses indentation-sensitive scoping. Key syntax elements:
- `::` — variable declaration (all variables are immutable outside declaration scope)
- `=>` — function call (postfix: `arg .fn_name =>`)
- `[]` — memory load/store
- `!` — bounds check operator
- `->` — branch/jump
- `` ` `` — format strings
- `@` — macros, `#` — directives
- `undefined`, `unreachable`, `unchecked` — explicit unsafe operations

See `specs-v4/` for the full language spec and `al-by-example/` / `from-other-by-example/` for usage examples.

## Adding a New Backend or Instruction

1. Add the declaration to `emit.h`.
2. Implement in both `emit-x86_64.c` and `emit-aarch64.c` (and `emit-ll.c` if applicable).
3. Call from `main.c` via the abstract interface — never call arch-specific functions directly from the parser.
