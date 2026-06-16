# Handoff: building an x86_64 PE32+ (Windows) binary backend for allang

This session built an **x86_64 ELF (Linux) binary backend** that emits a fully
linked, runnable executable directly (no `.s`, no external assembler/linker).
Before that, another session built the **arm64 Mach-O** backend the same way.
Your task is the analogous **x86_64 PE32+ (Windows) binary backend**: emit a
runnable `.exe` directly.

This document is the map: what exists, the architecture you must fit into, the
milestone order that worked, and — most valuable — the caveats that already cost
real time on the ELF and Mach-O backends, plus the ones unique to PE/Windows.

The good news up front: **PE dynamic linking is simpler than ELF's**, and the
x86-64 instruction encoder already exists and is reusable as-is. The real work is
the **container format** and the **Windows x64 ABI** (different from System V).

---

## 1. Goal / shape of the deliverable

Produce a backend pair, built the allang way (emit files passed explicitly):

```bash
./build.sh emit-x86_64-bin.c emit-pe-x86_64-exe.c       # <- your new container
./run.sh   emit-x86_64-bin.c emit-pe-x86_64-exe.c hello.al
```

- `emit-x86_64-bin.c` — the **machine-code emitter**. It **already exists** and
  is what the ELF backend uses. x86-64 machine code is identical regardless of
  target OS, so you reuse this almost wholesale. The one thing it bakes in that
  is *wrong* for Windows is the **ABI register allocation** (see §5, the central
  task). Plan to parametrize that, not rewrite the emitter.
- `emit-pe-x86_64-exe.c` — the **PE32+ executable container**: wraps the emitted
  code + data into a runnable `.exe` (DOS header → PE headers → sections →
  import table → entry stub). This is your main new file.
- `emit-bin.h` — the arch/OS-neutral **interface struct** between emitter and
  container. It already exists; reuse it. (`bin_image` carries
  `text/text_size/entry/imports[]/extcalls[]`.)

### Reference implementations to read first

The ELF backend is your closest analog. After the recent refactor it is split
into a **shared writer header** plus a thin **arch+OS glue file**:

- `emit_helper-elf-exe.h` — the OS-format-neutral ELF64 writer (layout math,
  headers, import/relocation tables, the `emit_output`/`bin_emit` plumbing). It
  is `#include`d by the glue file, not compiled on its own.
- `emit-elf-x86_64-exe.c` — defines the arch/OS-specific constants and the
  PLT/stub/patch builders, then `#include "emit_helper-elf-exe.h"`.
- `emit-elf-aarch64-exe.c` — the same for arm64 (proof the split generalizes).

**Mirror this split for PE from the start.** The ELF backend was first written as
one monolithic `emit-elf-exe.c` and had to be refactored into the
helper-`.h` + per-arch-`.c` shape afterward (the maintainer specifically wanted
this). Do it right the first time: `emit_helper-pe-exe.h` (the PE writer) +
`emit-pe-x86_64-exe.c` (the glue). Even though Windows-on-arm64 is out of scope
today, the split is what the maintainer expects and it keeps the format logic out
of the arch emitter.

Also read the existing **text** backend for Windows — it is your source of truth
for the **Win64 ABI** and PE assembler directives:

- `emit-x86_64.c` (x86 text codegen — ABI-neutral; it reads register arrays as
  `extern`).
- `emit-windows.c` (the Win64 ABI: `rname_param[]`, `rname_callee[]`, etc., and
  `output_ext = "s"`). **This file already encodes the exact register
  convention you must reproduce in the binary backend.**

---

## 2. The driver contract (emit.h) — unchanged, read it once

`main.c`'s `compile()` loop drives emission. Codegen accumulates into
backend-owned buffers; **one terminal hook (`emit_output`) writes the file.** No
hook writes the file mid-compile.

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

| hook | when | binary backend responsibility |
|------|------|-------------------------------|
| `emit_init()` | once, start | nothing (container) |
| `emit_fn_begin(ctx)` | per fn, before | reset `code`/`prol` buffers (emitter) |
| `emit_fn_end(ctx)` | per fn, after | concat `prol+body` into `code[]`, record symbol (emitter) |
| `emit_output(out)` | once, end | `bin_emit()` resolves fixups → container writes the whole image |

Because you reuse `emit-x86_64-bin.c`, the emitter half of this is **already
done**. Your container supplies `emit_init` + `emit_output` (and the
format-specific constants/builders the helper calls). `emit-x86_64-bin.c` exports
`bin_emit(bin_image*)`; your `emit_output` calls it and writes the `.exe`.

`emit.h` externs your container must define:
- `const char *output_ext;` — set to `"exe"` (the ELF/Mach-O exe backends use
  `""`; on Windows the runnable artifact wants the `.exe` extension).
- `const char *text_section_header;` / `string_section_header;` — set to `""`
  (binary backends don't emit assembler section text; the ELF exe files set both
  to `""`).
- `bool emit_need_escaping(void)` is already handled in the shared emitter
  (returns `true`); see caveat (C).

---

## 3. Milestone order (do them in this sequence)

Each milestone should end with every `tests/*.al` emitting a **well-formed PE**
(validated structurally; see §4 — you cannot run `.exe` in the cloud container).
Commit each milestone before the next.

1. **Exit-only — but this already needs the import table.** Unlike ELF (where a
   raw `syscall` gives you a libc-free static exit), **Windows has no stable
   syscall ABI**. Even exiting cleanly requires importing `ExitProcess` from
   `kernel32.dll`. So milestone 1 stands up the *entire* PE skeleton: DOS header,
   PE/COFF/optional headers, a `.text` section, an `.idata` import table with one
   DLL (`kernel32.dll`) and one import (`ExitProcess`), and an entry stub that
   calls `main` then `ExitProcess(retval)`. Targets: `return.al`,
   `ret_oneliner.al`. This is more up-front work than ELF milestone 1, but once
   the import table exists, milestone 4 (printf) is nearly free.
2. **Stack frames + locals.** The emitter already does this; your job is to make
   sure the **Win64 prologue** reserves shadow space and keeps 16-byte alignment
   (see §5). `nested.al`-style locals.
3. **Internal calls + string literals.** Already handled by the emitter
   (`call rel32` + RIP-relative `lea` for strings). Confirm the container places
   the string blob in a readable section (`.rdata` or just inside `.text` if it
   is RX; simplest is one RX `.text` holding code+strings, like the ELF static
   path). `forward.al`.
4. **External CRT calls (printf).** Add a second import DLL. On Windows, `printf`
   lives in the C runtime — `msvcrt.dll` is present on every Windows install and
   is the path of least resistance (`ucrtbase.dll` is the modern one but the
   import surface is fiddlier; start with `msvcrt.dll`). Because the IAT is the
   call target (see §5), this is just "add another import descriptor + thunks";
   there is **no PLT/GOT/hash-table machinery** to build. `hello.al`,
   `index_test.al`.
5. **Aggregates / arrays.** Already implemented in `emit-x86_64-bin.c`
   (`emit_eightbyte_struct`, `emit_store_*`, `emit_zerofill`,
   `emit_array_access`, `emit_elem_addr`). Should "just work" once 1–4 are solid.
   `array.al`, `expect.al`, `index_test.al`.

`emit_make_array` / `emit_store_array` are no-ops in every backend — leave empty.

---

## 4. "Done when" / how to verify (you cannot run the .exe here)

The cloud container has **no `wine`**, so you cannot execute the produced `.exe`.
This is exactly the situation CLAUDE.md already describes for aarch64: *"aarch64
emits assembly without error (no need to run it when you are in cloud
container)."* Treat PE the same way — **the bar in the container is "emit a
structurally valid PE32+ for every test without error,"** and actual execution is
verified on a real Windows host (or under `wine`/a Windows CI runner) by the
maintainer.

Tooling that **is** in the container, and how to use it:

- **Instruction encoding reference — works unchanged.** x86-64 bytes are
  OS-independent. The same workflow the ELF session used still applies:
  ```bash
  printf 'main:\n\tcall qword ptr [rip+0x10]\n\tsub rsp, 0x28\n' > ref.s
  clang -c -masm=intel ref.s -o ref.o
  objdump -d -M intel ref.o     # read the exact bytes
  ```
  e.g. `ff 15 10 00 00 00` = `call QWORD PTR [rip+0x10]` (indirect call through
  the IAT — your printf/ExitProcess call form), and `48 83 ec 28` =
  `sub rsp, 0x28` (40 = 32 shadow + 8 realign — the canonical Win64 prologue).
- **Produce a reference PE to learn the import-table layout.** There is no
  `mingw`/MSVC, but **`lld-link` and `clang --target=x86_64-pc-windows-msvc`**
  are present. You can assemble a tiny object and link a minimal PE to dissect.
- **Dissect any PE** with `llvm-readobj --coff-header --file-headers
  --coff-imports file.exe` and `llvm-objdump -d --target=pei-x86-64` (or
  `objdump`, which supports the `pei-x86-64` target). Use these to *learn* the
  exact field values a real linker writes, then reproduce them.
- **Validate your own output structurally** every milestone:
  `llvm-readobj --file-headers --coff-imports yourout.exe` should parse cleanly
  and show your imports. If `llvm-readobj` chokes, your headers are malformed.

As with ELF/Mach-O: inspection tools are a *learning aid and a smoke test*, not
the final oracle — the loader is. Where you can, have the maintainer run one
binary on Windows early to catch a class of "parses fine, won't load" bugs.

`examples/` is intentionally ill-formed — ignore it. Only `tests/` are valid.

---

## 5. PE32+ / Windows specifics (where the real work is)

### 5.1 The Win64 ABI is different from System V — this is the central task

`emit-x86_64-bin.c` currently **hardcodes System V register arrays**:

```c
static const uint8_t scratch_regs[] = { 0, 1, 2, 6, 7, 8, 9 };  // SysV
static const uint8_t callee_regs[]  = { 3, 12, 13, 14, 15, 10, 11 };
static const uint8_t param_regs[]   = { 7, 6, 2, 1, 8, 9 };      // rdi,rsi,rdx,rcx,r8,r9
static const uint8_t ret_regs[]     = { 0, 2 };
```

Windows x64 differs in three ways that **will** corrupt argument passing and
clobber callee-saved registers if ignored:

| | System V (current) | **Win64 (you need)** |
|---|---|---|
| Integer params | rdi, rsi, rdx, rcx, r8, r9 (**6**) | **rcx, rdx, r8, r9 (only 4)** `{1,2,8,9}` |
| Volatile/scratch | rax,rcx,rdx,rsi,rdi,r8,r9 | **rax,rcx,rdx,r8,r9,r10,r11** `{0,1,2,8,9,10,11}` |
| Callee-saved | rbx,r12–r15,r10,r11 | **rbx,rsi,rdi,r12–r15** `{3,6,7,12,13,14,15}` |
| Return | rax, rdx | rax, rdx (same) |

Note the trap: **rsi (6) and rdi (7) are volatile/param on SysV but
*non-volatile* on Win64.** If the emitter treats them as scratch, any call will
silently corrupt a caller's saved value.

**How to parametrize cleanly (follow the text backend's existing pattern).** The
*text* backend already solved this exact problem: `emit-x86_64.c` declares the
register arrays `extern` and the **OS file supplies them** — `emit-linux.c` gives
SysV names, `emit-windows.c` gives Win64 names. Reproduce that split in the
binary world:

- Change `emit-x86_64-bin.c`'s four register arrays (and their lengths) from
  `static const` to `extern const`, supplied by the exe/OS file.
- `emit-elf-x86_64-exe.c` defines them with the SysV values (move the current
  literals there).
- `emit-pe-x86_64-exe.c` defines them with the Win64 values above.

This keeps the arch emitter ABI-neutral, mirrors the text backend exactly, and
avoids forking the whole emitter into a `-win` copy. (If you prefer, a small
`abi` struct works too — but externs match precedent and the maintainer's taste.)

### 5.2 Shadow space + stack alignment (Win64 prologue)

Two hard ABI requirements the SysV prologue does **not** impose:

- **32-byte shadow space ("home space").** Before *every* `call`, the caller must
  reserve 32 bytes on the stack below the return address — the callee may spill
  its 4 register params there. Concretely, any function that makes a call must
  add 32 to its stack reservation.
- **16-byte alignment at the point of call.** `rsp` must be 16-byte aligned
  *before* the `call` instruction pushes the 8-byte return address. Since entry
  to a function leaves `rsp` at 8 mod 16 (the caller's `call` pushed 8), the
  prologue's `sub rsp, N` must make `N ≡ 8 (mod 16)` for a frame that itself
  calls out. The textbook minimal frame is `sub rsp, 0x28` (40): 32 shadow + 8
  realign. Reconcile this with `emit_fn_prologue_epilogue`'s stack-size math:
  round locals up and add shadow space so the final reservation keeps alignment.

`emit-windows.c` / the text path already gets this right; cross-check your byte
output against what the text backend reserves for the same function.

### 5.3 PE container layout (simpler than it looks)

A minimal PE32+ exe, in file order:

1. **DOS header** (`IMAGE_DOS_HEADER`, 64 bytes): starts with `MZ`; the only
   field that matters is `e_lfanew` (offset 0x3C) — the file offset of the PE
   signature. A DOS stub program is optional; you can point `e_lfanew` straight
   past the 64-byte header.
2. **PE signature** `"PE\0\0"`.
3. **COFF header** (`IMAGE_FILE_HEADER`, 20 bytes): `Machine = 0x8664`
   (IMAGE_FILE_MACHINE_AMD64), `NumberOfSections`, `SizeOfOptionalHeader`,
   `Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LARGE_ADDRESS_AWARE`.
4. **Optional header** (`IMAGE_OPTIONAL_HEADER64`): `Magic = 0x20B` (PE32+),
   `AddressOfEntryPoint` (an **RVA** to your entry stub), `ImageBase`
   (conventionally `0x140000000` for an exe), `SectionAlignment = 0x1000`,
   `FileAlignment = 0x200`, `SizeOfImage` (total virtual size, rounded to
   SectionAlignment), `SizeOfHeaders` (rounded to FileAlignment),
   `Subsystem = 3` (IMAGE_SUBSYSTEM_WINDOWS_CUI — **required for console stdout**),
   `NumberOfRvaAndSizes = 16`, and the **data directory** array. Only the
   **Import** directory (index 1, `IMAGE_DIRECTORY_ENTRY_IMPORT`) is mandatory.
5. **Section headers** (`IMAGE_SECTION_HEADER`, 40 bytes each): e.g. `.text`
   (RX, holds entry stub + code + strings), `.idata` (R, holds the import table).
   You can collapse strings into `.text`; keep `.idata` separate for clarity.
6. **Section bodies**, each padded to `FileAlignment` in the file and mapped at
   `SectionAlignment` in memory.

### 5.4 RVA vs file-offset duality — the #1 source of PE bugs

Everything in PE headers is expressed as an **RVA** (offset from `ImageBase`),
but the bytes live at **file offsets**. Because `FileAlignment (0x200)` ≠
`SectionAlignment (0x1000)`, the two diverge per section:
`rva = section.VirtualAddress + (file_off - section.PointerToRawData)`.
Write one helper that converts and use it everywhere (entry point, import
descriptor RVAs, IAT/ILT/hint-name RVAs, string RVAs). A single place that
confuses the two will produce a PE that `llvm-readobj` parses but the loader
rejects. This is the PE analog of ELF's vaddr-vs-offset bookkeeping, but more
error-prone because of the alignment mismatch.

### 5.5 Imports: the IAT *is* the GOT — no PLT, no resolver

This is where PE is **simpler than ELF**, and it directly retires the most
elaborate machinery the ELF session built (PLT stubs, `.dynsym`/`.dynstr`,
ELF hash table, `.rela.plt`, the lazy-vs-BIND_NOW question). For PE:

- The **Import Directory** is a null-terminated array of `IMAGE_IMPORT_DESCRIPTOR`
  (one per DLL): `OriginalFirstThunk` (RVA → ILT), `Name` (RVA → `"kernel32.dll"`),
  `FirstThunk` (RVA → IAT).
- **ILT** (Import Lookup Table) and **IAT** (Import Address Table) are *parallel*
  arrays of 64-bit thunks, both null-terminated. For import-by-name, each thunk
  holds an RVA to an `IMAGE_IMPORT_BY_NAME` (2-byte hint `0` + the
  null-terminated symbol name). (High bit set would mean import-by-ordinal; use
  by-name.)
- At load time the loader **overwrites the IAT** in place with the resolved
  function addresses. So at runtime, the IAT slot holds the real function
  pointer — exactly like a fully-resolved (BIND_NOW) GOT entry.
- **Calling an import is a single indirect call through the IAT slot:**
  `ff 15 <disp32>` = `call QWORD PTR [rip + (IAT_slot_rva - rip_next_rva)]`.
  No PLT stub, no lazy resolver, no `GOT[0..2]` reserved slots. The emitter
  leaves external `call` sites as `extcalls`; your container patches each into a
  `ff 15` whose disp32 points at that import's IAT slot.

  > Callback to an ELF lesson: the ELF backend agonized over lazy binding vs.
  > `DF_BIND_NOW` and ultimately matched gcc/clang's *lazy* PLT. **On PE the
  > question simply doesn't exist** — the IAT is always bound at load. One fewer
  > decision.

Practical tip: the emitter currently emits external calls as `e8 <rel32>` (direct
`call rel32`) and reports the site as an `extcall`, because that is what ELF PLT
stubs want. PE wants `ff 15 <rel32>` (indirect), which is **2 bytes longer**
(opcode `ff 15` vs `e8`). Decide how to reconcile this:
  - cleanest: have the emitter emit the call site in a form the container can
    patch to either shape, or reserve the larger encoding; **or**
  - emit a tiny 6-byte stub per import in `.text` (`ff 25 <iat_rva>` =
    `jmp [rip+IAT]`, an "import thunk" exactly like MSVC's `__imp_` stubs) and
    keep the call sites as `e8 <rel32>` to the stub. The second option changes
    **nothing** in the emitter and localizes all PE-ness in the container — it is
    the lower-risk path and mirrors how the ELF container already turns `e8`
    sites into PLT entries. Recommended.

### 5.6 Entry stub + the stdio-flush trap (a known carryover bug)

Provide your own entry (the CRT's `mainCRTStartup` does not exist in a hand-built
exe). `AddressOfEntryPoint` → a stub that calls allang `main`, then exits:

```
    sub  rsp, 0x28            ; shadow space + alignment
    call main                 ; e8 rel32  (or through your import-thunk scheme)
    mov  ecx, eax             ; exit code -> 1st Win64 arg (rcx)
    call [rip+__imp_ExitProcess]   ; ff 15
```

**The stdio-flush trap is the same bug the ELF session hit.** On ELF, exiting via
the raw `exit_group` syscall (or `_exit`) **silently swallowed `printf` output**
because libc's stdio buffer was never flushed; the fix was to route the exit
through libc's `exit@plt`. The identical hazard exists on Windows:
**`ExitProcess` does NOT flush the C runtime's stdio buffers.** If `hello.al`'s
output is buffered in msvcrt and you terminate with `ExitProcess`, the text never
appears. Mitigations, in order of cleanliness:
  - import and call msvcrt's **`exit`** (which runs CRT atexit/`_cexit` flushing)
    instead of `kernel32!ExitProcess` once you are already linking msvcrt for
    `printf`; **or**
  - call **`fflush(NULL)`** (msvcrt) before `ExitProcess`; **or**
  - for the libc-free milestone-1 (no printf yet), `ExitProcess` alone is fine.

Flag this early: the symptom ("program exits 0 but prints nothing") is identical
to the ELF bug and wasted real time there.

### 5.7 No `<windows.h>` on the Linux build host — declare the structs yourself

The ELF session was told to use `<elf.h>` (a system header on Linux) instead of
hand-rolled structs. **There is no equivalent for PE** — the Linux build host has
no `<windows.h>`/`<winnt.h>`. So hand-declaring the `IMAGE_*` structs is
unavoidable and *expected* here. Mitigate the downside the same way `<elf.h>`
does: **name every struct and constant exactly as the Win32 SDK does**
(`IMAGE_DOS_HEADER`, `IMAGE_NT_HEADERS64`, `IMAGE_FILE_HEADER`,
`IMAGE_OPTIONAL_HEADER64`, `IMAGE_SECTION_HEADER`, `IMAGE_IMPORT_DESCRIPTOR`,
`IMAGE_IMPORT_BY_NAME`; constants `IMAGE_FILE_MACHINE_AMD64`,
`IMAGE_SUBSYSTEM_WINDOWS_CUI`, `IMAGE_SCN_MEM_EXECUTE`, etc.). Keep the subset
minimal — only the fields you set — but use `#pragma pack(push,1)` / explicit
`uint8/16/32/64_t` so the on-disk layout is exact, and **name the magic numbers**
(the maintainer rejected bare hex like `0x8664`/`0x20b` on the ELF backend; use
named constants).

---

## 6. Caveats that already cost time (carried over — they still apply)

**(A) Re-entrant `compile()` via `#compile_all`.** Several tests start with
`#compile_all expect.al`, which calls `compile()` **recursively** between an outer
`main`'s `emit_fn` and its `emit_fn_end`. This is **already handled in
`emit-x86_64-bin.c`** (entry/symbol offsets recorded at `emit_fn_end` time using
the true landing offset; per-function state on the C stack in `emit_context_t`;
label/fixup relocation via a `done` flag, not an index range; buffers zeroed at
`emit_fn_end`). You inherit the fix for free **as long as you reuse the emitter**
— but if you touch `emit_fn`/`emit_fn_end`, preserve all four behaviors. Symptom
if you break it: the binary's entry points at the *included* file's first
function, so the program runs but exits with the wrong code.

**(B) Prologue-after-body ordering.** The parser emits the body before the final
stack size is known, so `emit_fn_prologue_epilogue` runs *after* the body. The
emitter handles this with two buffers (`prol[]` + `body[]`) concatenated in
`emit_fn_end`. Already done — but it is exactly where your **Win64 shadow-space /
alignment** change (§5.2) lands, so understand it before editing the prologue.

**(C) `emit_need_escaping() == true`.** Binary backends unescape string literals
themselves. This is already `true` and the shared frontend escape path is already
fixed. If you hit a string-related crash, **fix the real bug in the shared
frontend, not a backend-local workaround** (standing maintainer preference).

**(D) Don't trust inspection tools as the oracle — but here you can't run it
either.** `llvm-readobj`/`objdump` may accept a hand-built PE the loader rejects
(and vice-versa). Since you also can't execute `.exe` in the container, get one
real run on Windows/wine as early as possible (milestone 1's exit-only exe is the
cheapest thing to validate end-to-end).

**(E) Capacities.** The ELF session bumped `LABEL_CAP`/`FIXUP_CAP` to 1024
(`index_test.al` overflowed at 256). Those live in `emit-x86_64-bin.c` and are
already sized; if you add per-import tables in the container, size them with the
same generosity and add a `report_error` overflow guard.

**(F) Coding conventions (enforced — the ELF session was corrected on these).**
- **No comments that describe what the code does.** Function names and the code
  itself already say that. A comment is justified *only* for a non-obvious WHY: a
  hidden constraint, a subtle invariant, a workaround. The ELF session added
  "what" comments (`/* PLT[0]: push [GOT[1]]; jmp [GOT[2]] */`) and had to strip
  every one. Default to **zero** comments.
- **Name magic numbers** with `#define`/`const` (no bare `0x8664`, `0x20b`,
  `0x140000000`).
- **Const pointers** wherever possible.
- **`return` on its own line** — never `if (cond) return x;`.
- **Split the container** (`emit_helper-pe-exe.h` + `emit-pe-x86_64-exe.c`) from
  the start; don't write a monolith and refactor later (§1).

---

## 7. Suggested commit / milestone map (mirror the ELF cadence)

The ELF backend landed roughly one commit per milestone, each green before the
next. Aim for the same:

1. PE skeleton + import table + `ExitProcess` + exit-only (`return.al`,
   `ret_oneliner.al`). Also: parametrize the ABI register arrays (extern from the
   OS file) and define the Win64 set here — do this in milestone 1 so every later
   milestone runs on correct register allocation.
2. Win64 prologue (shadow space + alignment); locals.
3. Internal calls + string literals in a mapped section.
4. printf via `msvcrt.dll` (second import) + the stdio-flush fix (§5.6).
5. Aggregates/arrays (should fall out of the reused emitter).

Read `emit-elf-x86_64-exe.c` + `emit_helper-elf-exe.h` end-to-end first — your
container is the same shape with PE structures substituted for ELF ones, and
**less** dynamic-linking machinery (no PLT/hash/dynsym; the IAT is the GOT). The
emitter (`emit-x86_64-bin.c`) you reuse as-is except for making the four register
arrays `extern`. Start at milestone 1, validate each PE with `llvm-readobj`, and
verify every instruction's bytes against `clang`+`objdump` before building on it.
