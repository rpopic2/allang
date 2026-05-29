# should be make_struct handled by backend?

when handled by backend:
* we need to write make_struct for each architecture.

pros:
* we can handle if the architecture has different memory layout.. -> but it dosn't

i think it is better to be handled from frontend if possible.


# how should be branched returns handled? how should be return statement compiled to?
in traditional languages, return statemnet dosn't exactly match to ret instruction because of epilogues.
it could mean few things:
* ret instruction if there isn't epilogue.
* branch to epilogue
* has copied epilogue

we want to optimise this...

case 1:
```asm
cmp x0, x1
b.gt skip
mov w0, 0
b epilogue
skip:
; do sth..
```

can be turned into one branch
case 2:
```asm
cmp x0, x1
mov w0, 0
b.le epilogue
; do sth..
```

this optimization cannot be used if we should not destroy x0.

it may be better to have two rets if there is no epilogue:
case 3:
```asm
cmp x0, x1
b.le skip
mov w0 0
ret
skip:
; do sth
ret
```

## what we actually do today
every `ret` lowers to a branch to a single shared label `.<fn>.ret`
(`emit_branch` / `emit_branch_cond` in main.c). at the end of the function the label is
emitted once, then the epilogue (`emit_fn_prologue_epilogue`), then one `emit_ret`.
so today everything is **case 1**. the spec agrees: returns "jump to the return block,
not actually compiling to ret" (specs-v4/asm/branches.md).

## the three cases are two orthogonal axes
the three snippets are not one mutually-exclusive choice. they are two independent
decisions:

* **epilogue sharing (case 1 vs case 3).** do N return sites *branch to one shared
  epilogue*, or *inline epilogue+ret* at each site? this is decided purely by epilogue
  size.
  * non-empty epilogue (callee-saved restores + frame teardown): sharing wins on code
    size as soon as there are >=2 returns -> case 1.
  * empty epilogue (leaf fn: no callee-saved, no frame, no calls): the shared "block" is
    a lone `ret`, so branching to it is pure overhead -> case 3 (inline ret). the trigger
    is exactly the leaf check we already have (`!calls_fn && max_nreg_count == 0 &&
    stack_size == 0`), and both backends already emit nothing for it.
* **guard fusion (case 2).** for a *conditional* return `cond -> ret v`, invert the
  guard so the conditional branch jumps straight to the return target. this collapses the
  skip-branch + unconditional epilogue jump (two branches) into one. it is a peephole
  layered on top of the other axis, independent of epilogue size.

## decision
case 1 stays as the **fallback**. but case 2 is the **preferred** lowering for
conditional returns wherever it is provably safe, and case 3 is used for unconditional
returns in empty-epilogue (leaf) functions.

* `cond -> ret v` that passes the guards below -> **case 2**.
* `cond -> ret v` that fails a guard -> **case 1**.
* unconditional mid-body `ret` -> branch to `.ret` (case 1); but in a leaf/empty-epilogue
  fn emit an inline `ret` instead (**case 3**).
* tail / fall-off return -> fall straight into epilogue+ret, no branch (as today).

note case 2 *dominates* case 3 for conditional returns even in leaf functions: one
inverted conditional branch to a single `ret` beats a skip-branch + duplicated `ret`. so
case 3's real domain is *unconditional* returns in leaf functions.

## when case 2 is safe (preconditions)
1. **return-register liveness.** hoisting the value write puts it on both paths, so it is
   only safe if the return register is dead on the fall-through. in our convention RET
   regs are scratch-like and named values live in NREG (callee-saved), so RET is usually
   dead across; the dangerous case is when the previous `=>` call's result is still in RET
   and the fall-through reads it. single-pass proxy: keep a "RET-dirty" flag, set when a
   call result lands in RET and cleared when consumed; allow case 2 only when clean. (this
   is the precise version of the earlier "unless we should not destroy x0" / "unless the
   previous statement was a function returning a value".)
2. **flag neutrality / ordering.** the value materialization must not clobber the guard's
   flags. fix the order to **value -> cmp -> branch** (materialize *above* the compare). if
   the value can only come from a flag-setting op that also depends on the compared
   operands, reject -> case 1.
3. **simple invertible guard.** single comparison only (trivial inverse via COND_EQ/NE/
   GE/LT). compound `and`/`or` guards need De Morgan + short-circuit restructuring, so they
   fall back to case 1 for now.
4. **body is exactly `ret v`** (the one-liner). multi-statement guarded blocks just get
   ordinary inverted branch-over lowering, not return fusion.

## x86_64 notes
* `cmp`+`jcc` macro-op-fuse into one macro-op on modern x86_64, so case 2 is a strong win
  there -- *but only if* the value `mov` is hoisted above the `cmp`. inserting it between
  cmp and jcc breaks fusion. this is why precondition 2 fixes the order as value -> cmp ->
  branch. corrected x86_64 case 2:
```asm
mov eax, 0      ; value first, keeps cmp+jcc adjacent
cmp edi, esi
jle .fn.ret     ; fused, inverted condition -> shared epilogue
; do sth..
```
* `ret` pops the return address on x86_64 (uses x30 on aarch64). duplicating `ret`
  (case 3) keeps each `ret` paired with a `call`, so the return-address-stack predictor
  stays balanced on both -- case 3 is perf-safe.
* encoding sizes: short jmp/jcc = 2 bytes, `ret` = 1 byte. for N leaf return sites case 3
  ~= N bytes vs case 1 ~= 2N+1; for a non-empty epilogue of size E, case 1 (2N+E) beats
  case 3 (N*(E+1)) at N>=2 -- the axis-1 rule comes out the same on both arches.
* x86_64 already emits no prologue/epilogue under the same leaf predicate, so the case 3
  trigger is identical on both backends and the decision stays in the frontend.

## single-pass implementation notes
* case 2 needs to know the guarded block is a lone `ret` at the point the guard branch is
  emitted -> one-line lookahead in the parser (the conditional-return path already lives
  near the `is`/`->` handling and `stmt_ret_cond`). inversion is trivial; liveness uses the
  RET-dirty proxy.
* case 3 needs leaf-ness, which is only final at function end (a later `=>` can appear).
  the whole function is buffered in `fn_buf` and flushed at finalize, so this is a
  finalization-time backend decision: if the epilogue turned out empty, lower buffered
  unconditional return points as inline `ret` instead of `b .ret`.

## `leaf` keyword (avoids the case-3 finalize step)
instead of inferring empty-epilogue at finalize, declare it in the signature with a
`leaf` keyword. this turns the whole-function property into an up-front, compiler-checked
contract, so the case-3 decision moves from finalization to emit time.

* **what it asserts: empty epilogue.** `leaf` means the function has *no frame at all*:
  no `=>` calls, no callee-saved / NREG usage, no stack locals -- exactly the existing
  predicate `!calls_fn && max_nreg_count == 0 && stack_size == 0`. note this is stricter
  than the textbook "leaf = makes no calls": a no-call function can still use named
  registers or locals and thus still need an epilogue. here the keyword carries the
  stronger no-frame meaning, so a `leaf` function is restricted to scratch registers and
  params only.
* **it is checked, not just a hint.** because the contract is declared first, every `ret`
  in the body can emit an inline `ret` (case 3) immediately, with no finalize-time
  rewrite. if the body then introduces a call, a named register, or a local, that is a
  compile error -- a `leaf` fn that needs a frame is rejected. this also makes it a useful
  explicit perf guarantee for hot code, matching the explicit-marker ethos
  (`undefined` / `unreachable` / `unchecked`).
* **un-annotated functions are unaffected.** they keep case 1 (or the optional
  finalize-time inference above). the keyword only opts a function into guaranteed,
  emit-time case 3.
* **orthogonal to case 2.** `leaf` answers only the whole-function epilogue question. it
  says nothing about whether a given `cond -> ret v` block is a lone `ret`, which is a
  per-statement fact. so case 2 still needs its own one-line lookahead and remains the
  preferred lowering for conditional returns, independent of `leaf`. (and inside a `leaf`
  function a conditional return is still better as case 2 -- one inverted branch to a
  single `ret` -- than as a skip-branch plus duplicated `ret`.)


# from where should we be traking registers? how to handle 16-bit register?
# how to deal with msvc x64 calling convention?
# how to deal with x86_64 having only half register than aarch64?
we cannot have named registers as much as we do in aarch64.
callee-saved registers in..
aarch64: 19~27 (8 regs)
x86_64: 7 regs in windows, 5 regs in linux. si, di is used as scratch in linux.. for function args.
-> let's have our own calling convention. use other only if is exported.

allow 7 named registers.

if you look from the frontend.. 16-bit registers should take up two registers even from the frontend.


backend should care about hardware-specific optimizations, while frontent should care about generic optimizations. if anything is generic and common for aarch64 and x86_64, and macos/linux/windows, it should be in the frontend.
