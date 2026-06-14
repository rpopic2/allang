# Handoff: building an x86_64 ELF binary backend for allang

This session built a **macOS arm64 Mach-O binary backend** that emits a fully
linked, runnable, code-signed executable directly (no `.s`, no external
assembler/linker). Your task is the analogous **x86_64 ELF (Linux) binary
backend**. This document is the map: what exists, the architecture you must fit
into, the milestone order that worked, and the caveats that cost real time.

---

## 1. Goal / shape of the deliverable

Produce a backend pair, built the allang way (emit files passed explicitly):

```bash
./build.sh emit-x86_64-bin.c emit-elf-exe.c       # <- your new files
./run.sh   emit-x86_64-bin.c emit-elf-exe.c hello.al
```

- `emit-x86_64-bin.c` — the **machine-code emitter**: implements every
  `emit_*()` in `emit.h` by appending x86-64 bytes to a module buffer.
- `emit-elf-exe.c` — the **ELF executable container**: wraps the emitted code +
  data into a runnable ELF64 file. (A future `emit-elf-reloc.c` could swap in to
  produce `.o` files; defer it.)
- `emit-bin.h` — the **interface struct** between them. It already exists and is
  arch/OS-neutral; reuse it, extend only if needed.

The arm64 equivalents to read as your reference implementation:
- `emit-aarch64-bin.c` (emitter) and `emit-macho-exe.c` (container).
- `emit-bin.h` (the shared `bin_image`/`bin_import`/`bin_extcall` structs).

Also read the existing **text** backends, which already do correct codegen and
are your source of truth for *what instructions to emit per op*:
- `emit-x86_64.c` (x86 text — port its logic 1:1; it emits AT&T/Intel asm).
- `emit-linux.c` (the ELF/Linux OS preamble for the text path).

There is already a `emit-elf.c` — a *monolithic* (not split) direct-ELF stub
that writes a fixed `exit(0)` ELF and no-ops every instruction emitter. Treat it
as a starting sketch of the ELF64 header/program-header layout, but the clean
shape is the split `emit-x86_64-bin.c` + `emit-elf-exe.c` mirroring the arm64
pair. It already conforms to the new lifecycle (`emit_output` writes the file).
Decide early whether to grow `emit-elf.c` in place or supersede it with the
split; the split is recommended for parity with arm64.

---

## 2. The driver contract (emit.h) — read this before anything else

`main.c`'s `compile()` loop drives emission. The contract is intentionally
simple: **codegen accumulates into backend-owned buffers; one terminal hook
writes the finished output file.** No hook writes to the file mid-compile.

```c
emit_init();                    // backend setup
while (...) {                   // one iteration per top-level function()
    emit_context_t emit_ctx = {0};
    emit_fn_begin(&emit_ctx);
    function(&src);             // parser emits emit_*() calls here
    emit_fn_end(&emit_ctx);     // finalize this fn INTO the backend's buffer
}
emit_output(out);               // the ONLY writer: serialize everything to file
```

(This was recently refactored. The hooks used to be named after text-assembler
sections — `emit_text` wrote the `.text` header before compile, `emit_cstr`
wrote the `.cstring` section after — and the binary backend abused `emit_cstr`
to write the whole executable. That asymmetry is gone; see the lifecycle table
below.)

| hook | when | text backend | binary backend |
|------|------|--------------|----------------|
| `emit_init()` | once, start | init buffers, stage section headers | nothing |
| `emit_fn_begin(ctx)` | per fn, before | push ctx, init fn buffers | reset `code`/`prol` buffers |
| `emit_fn_end(ctx)` | per fn, after | append fn asm to `text_buf` | concat `prol+body` into `code[]`, record symbol |
| `emit_output(out)` | once, end | write `text_buf` + cstring buf to file | serialize/link/(sign) the whole image to file |

Consequences for the binary backend design:

- **No hook writes the file until `emit_output`.** During the `compile()` loop
  the **emitter** accumulates instruction bytes into a module-level buffer
  (`code[]`) plus side tables (labels, fixups, string literals, imports). Then
  `emit_output` calls a single `bin_emit(bin_image*)` (exported by the emitter)
  that resolves all fixups and hands the container back
  `{text, text_size, entry, imports, extcalls}`, and the container writes/links/
  (signs) the whole file.
- `emit_fn_begin` / `emit_fn_end` live in the **emitter** (it owns the code
  buffer); `emit_init` / `emit_output` live in the **container** (it owns the
  file format). `emit_fn_end` takes **no `FILE*`** — it only finalizes into the
  buffer.
- `emit_fn(str name)` (parser-facing: "a function with this name starts here")
  is separate from the `emit_fn_begin`/`emit_fn_end` lifecycle pair — don't
  confuse the three.

`emit.h` extras you must satisfy:
- `extern const char *output_ext;` — set it in your container. arm64 exe uses
  `""` (bare executable, no extension). Linux text backends use `"s"`.
- `bool emit_need_escaping(void)` — see caveat (C) below; return `true`.

---

## 3. Milestone order that worked (do them in this sequence)

Each milestone ended green and was committed before the next. Mirror this.

1. **Exit-only.** Get `tests/return.al`, `ret_oneliner.al` to produce a runnable
   ELF that exits with the right code. This is where you nail the ELF container
   skeleton + `mov`/`add`/`sub`/`cmp`/`ret`/`mov_reg`/branches/labels/`cset`.
2. **Stack frames + locals.** `emit_fn_prologue_epilogue`, `emit_str_reg`/
   `emit_ldr`/`emit_str_imm` (load/store to frame slots). Run programs with
   locals.
3. **Internal calls + string literals.** `emit_fn_call` (call + fixup resolved
   against a function-symbol table populated in `emit_fn`), and `emit_string_lit`
   (RIP-relative `lea` to a string blob appended after code; PC-relative fixup).
   `tests/nested.al`, `forward.al`, `hello.al` (string printed via... see #4).
4. **External libc calls (printf).** This is the heavy one on ELF — see §5.
5. **Aggregates / arrays.** `emit_eightbyte_struct` (+ helpers), `emit_store_*`,
   `emit_zerofill`, `emit_array_access`, `emit_elem_addr`, register-offset
   loads/stores, and **width-correct** `add/sub/cmp`/`mov_reg` (zero/sign
   extension for mixed operand widths). Unblocks `array.al`, `expect.al`,
   `arithmetic.al`, `index_test.al`. (This was the last milestone done this
   session for arm64 — port `emit-x86_64.c`'s versions.)

`emit_make_array` / `emit_store_array` are **no-ops** even in the text backends —
leave them empty.

---

## 4. "Done when" / how to verify

- `test-all-asm.sh` must pass. **Important:** it builds the *default text* backend
  and only checks **exit code 0** — it does **not** diff stdout, and the
  `tests/*.ok` files are **empty/vestigial**. So "pass" == "compiles + exits 0".
- Your binary backend is validated *separately*: build it explicitly, then for
  each `tests/*.al` confirm it produces a runnable ELF that exits 0 (and prints
  the expected text for `hello`/`index_test`/etc.). The arm64 sweep script:
  ```bash
  ./build.sh emit-x86_64-bin.c emit-elf-exe.c
  for al in tests/*.al; do t="${al%.al}"; ./alc "$al" >/dev/null 2>&1
    file "$t" | grep -q ELF && { ./"$t"; echo "$(basename $t) rc=$?"; }; done
  ```
- `examples/` contains intentionally ill-formed code — ignore it. Only `tests/`
  are valid programs.

**Verify every instruction encoding against the assembler before trusting it.**
The workflow that kept us correct:
```bash
cat > ref.s <<'EOF'
  mov  rax, 60
  lea  rax, [rip+0x10]
  ...
EOF
clang -c ref.s -o ref.o      # or: gcc -c
objdump -d ref.o             # read the exact bytes, copy them into a macro
```
On arm64 we did this for *every* opcode and stored each as a named macro. Do the
same for x86-64. **Use named `#define OP_*` macros for opcodes, not bare hex** —
the user explicitly required this for readability.

---

## 5. x86_64 / ELF specifics that differ from arm64 (where the real work is)

### Instruction encoding is variable-length (1–15 bytes)
This is the biggest structural difference. The arm64 backend indexed everything
by **32-bit word** (`code[]` is `uint32_t[]`, fixup sites are word indices). On
x86-64 you must work in **byte offsets**: `code[]` is a `uint8_t[]`, fixup sites
are byte offsets, branch displacements are computed in bytes. REX prefixes,
ModRM, SIB, and disp8-vs-disp32 selection all matter. Budget time for this.

- Branches/calls use **rel32** displacements relative to the *end* of the
  instruction. Plan fixups accordingly.
- String/global access uses **RIP-relative** `lea reg, [rip+disp32]` where
  disp32 is relative to the *next* instruction. The arm64 `adr` fixup logic
  maps to this but with end-of-instruction-relative math.

### ELF container is simpler than Mach-O in some ways, harder in others
- **No code signature.** Linux does not SIGKILL unsigned binaries (macOS does —
  that whole CommonCrypto/CodeDirectory machinery in `emit-macho-exe.c` has *no*
  ELF analog; delete that concern).
- **Static exit-only ELF is trivial and fully allowed:** one `PT_LOAD` segment
  (R+X) covering ELF header + program headers + code, `e_entry` pointing at your
  entry. You must provide your own `_start` (the process entry): allang `main`
  returns a value in a register; `_start` should call `main` then do
  `exit_group` via syscall (`mov rax, 231; mov rdi, <code>; syscall`) — or
  `exit` (`rax=60`). There is **no libc/crt** in a hand-built static ELF, so a
  bare `ret` from the entry will crash. (On macOS the analog was: static
  raw-syscall binaries are *rejected*; ELF is the opposite — static raw syscalls
  are the easy path.)
- **Dynamic linking for printf is the hard part.** Unlike Mach-O chained
  fixups, ELF needs: a `PT_INTERP` segment naming the loader
  (`/lib64/ld-linux-x86-64.so.2`), a `PT_DYNAMIC` segment, and the
  `.dynamic`/`.dynsym`/`.dynstr`/`.rela.plt`/`.got.plt` tables, plus `DT_NEEDED`
  for `libc.so.6` and PLT stubs. Relocations are `R_X86_64_JUMP_SLOT` (PLT) /
  `R_X86_64_GLOB_DAT`. This is a *different* mechanism from arm64's
  `LC_DYLD_CHAINED_FIXUPS`; do not try to port it byte-for-byte — re-derive it
  by disassembling/`readelf -a` a tiny `gcc`-linked `printf` binary.
  - **Strategy tip:** consider whether your test programs can be satisfied with
    raw `write(2)`/`exit(2)` syscalls first (milestone 1–3 need no libc at all),
    and only stand up the PLT/dynamic machinery for the printf milestone. That
    keeps you green for longer.
- `bin_image` already carries `imports[]` + `extcalls[]`; the emitter leaves
  external `call` sites unpatched and reports them, the container builds PLT
  stubs and patches the `call rel32` to the stub. Same division of labor as
  arm64 — reuse the interface.

### Register mapping
The arm64 emitter has `reg_no(reg_t)` translating allang's abstract registers
(`SCRATCH`, `NREG`, `FRAME`, `STACK`, `RD_NONE`, `RET`/`PARAM`) to physical regs.
Build the x86-64 equivalent from `emit-x86_64.c`'s existing register convention
(it already picks rdi/rsi/... for params, a callee-saved set, rbp frame, rsp).
Match the text backend's choices exactly so behavior is identical.

---

## 6. Caveats that cost time (read these — they are not obvious)

**(A) Re-entrant `compile()` via `#compile_all`.** Several tests start with
`#compile_all expect.al`, which calls `compile()` **recursively** in the middle
of the outer `main`'s `function()` call — i.e. *between* the outer `emit_fn` and
its `emit_fn_end`. This breaks any per-function state captured in fixed globals
at `emit_fn` time. Two fixes were required and you must replicate them:
  - **Entry point + function symbol offsets must be recorded at `emit_fn_end`
    time**, using the *true* landing offset (`code_len` at finalize), NOT at
    `emit_fn` time. We captured it at `emit_fn` time → `entry` was a stale `0`
    → the binary's entry pointed at the *first-emitted* function (the included
    file's, not `main`). Symptom: program runs but `exit 1` / wrong behavior.
  - **Per-function frame state lives on the C stack**, in `emit_context_t` (the
    driver allocates one local per loop iteration). We added `str fn_name;
    bool fn_named;` to `emit_context_t` in `emit.h`; `emit_fn_begin` clears
    them, `emit_fn` writes them (via a single `static emit_context_t *active_ctx`
    cursor, since `emit_fn` has no context param), `emit_fn_end` reads them off
    its own `context` arg. No fixed-size frame array — the real recursion
    handles depth. (We first used a `static fnframes[32]` array; the user asked
    us to move it onto the stack frame, which is cleaner.)
  - **Label/fixup relocation must use a `done` flag, not a start-index range.**
    Because the outer `main`'s labels are split *around* the nested compile, a
    `[fn_start, n)` range double-relocates the nested file's labels. Instead,
    give each label/fixup a `bool done`; each `emit_fn_end` relocates only
    not-yet-done entries by that function's delta, then marks them done.
  - `emit_fn_end` should also **zero the per-function code/prologue buffers at
    its end**, so the outer `main` resumes into a clean buffer after the nested
    compile returns.

**(B) Prologue-after-body ordering.** The parser emits the body before it knows
the final stack size / saved-register set, so `emit_fn_prologue_epilogue` runs
*after* the body. arm64 solved this with two buffers: emitters append to
`body[]`, prologue helpers to `prol[]`; `emit_fn_end` concatenates
`prol` then `body` into the global `code[]` and relocates that function's labels
and fixups by `delta = base + prol_len`. Replicate this (in byte units for x86).

**(C) `emit_need_escaping()==true` exposed latent frontend bugs.** The text
backends return `false` (the assembler unescapes string literals); a binary
backend must unescape itself, so it returns `true`. That flipped on a
previously-unused code path in `main.c`/`str.h` that had two real bugs. Those are
**already fixed in shared code** (don't re-fix), but: keep `emit_need_escaping()
== true`, and if you hit a string-related crash, suspect the shared escape path,
and **fix the real bug in the shared frontend rather than adding a backend-local
workaround** (this is a standing user preference).

**(D) Don't trust inspection tools as the validity oracle — run the binary.**
On macOS, `dyld_info` *rejected* our valid binaries (it wanted legacy load
commands) even though the loader ran them fine. The ELF analog: `readelf`/
`objdump`/`eu-elflint` may complain about a hand-built ELF that the kernel +
`ld.so` nonetheless load and run. Use them to *learn* the layout from a real
`gcc` binary, but judge correctness by **running** your output.

**(E) Capacities.** We had to bump `LABEL_CAP`/`FIXUP_CAP` (256 → 1024);
`index_test.al` overflowed. Size yours generously and add an overflow
`report_error` guard.

**(F) Coding conventions (enforced).** No comments unless strictly necessary
(and then one short line, justified). Const pointers where possible. `return` on
its own line. Replace hardcoded opcode hex with named `#define` macros.

---

## 7. Quick file/commit map of what this session produced (arm64)

- `b6ab2aa` split + real codegen + prologue/epilogue
- `434a90c` internal calls + string literals
- `e7845b5` `output_ext` (backend chooses extension) + moved `report_error` to a
  single def in `main.c` (declared in `err.h`)
- `9e8b22b` external dylib calls (real `printf` via libSystem + chained fixups)
- `bd534ef` aggregate/array ops + extended-width arithmetic + the re-entrancy fix
  (done-flag relocation, finalize-time entry/symbol recording)
- `ec544ca` moved per-function frame state off a static array onto the C stack
  (`emit_context_t` fields)
- `31201a9` reconciled the emit lifecycle: `emit_reset_fn`→`emit_fn_begin`,
  `emit_finalize_fnbuf`→`emit_fn_end` (no `FILE*`), removed `emit_text`,
  `emit_cstr`→`emit_output` (sole writer). Both backends now accumulate during
  codegen and serialize once in `emit_output`. This is the interface §2 above
  describes; the older commits used the pre-rename names.

Read `emit-aarch64-bin.c` end-to-end first; it is ~750 lines and every emit_* op
maps to one you must implement. Your x86-64 versions come from `emit-x86_64.c`.
The container is where ELF and Mach-O diverge most — `emit-macho-exe.c` shows the
*shape* (header → segments/sections → entry/symbol patching → write), but the ELF
contents are entirely different and simpler (no signing).

Start at milestone 1, keep `test-all-asm.sh` green throughout, and verify each
opcode's bytes against `objdump` before you build on it.
