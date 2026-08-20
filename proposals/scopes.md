## scopes

Assessment of the scope rules, checked against `main.c` at c011128.

Everything under "verified" was run through `alc`; the transcripts are
reproducible from the snippets shown.

---

### 1. what is actually enforced today

| rule | status | evidence |
| --- | --- | --- |
| block scope ends a name's lifetime | **enforced** | `W ::` inside `I is 0 ->`, used after the dedent → `unknown id W` |
| line scope ends a name's lifetime | **enforced** (vacuously) | a one-liner block cannot declare at all |
| label region ends a name's lifetime | **not enforced** | `X ::` between `loop:` and `loop.break:` is still live after `loop.break:` |
| object modified only in its initializing block | **not enforced** | `X :: i32{7}` then `i32{9} =X` compiles clean and takes effect |
| `&` mutable sigil | **not implemented** | grammar §10 |
| block scope generates a break label | **partly** | only `cond ->` blocks emit one (`lbb<N>`); `::` blocks emit none; a fn emits `.ret` only when `has_branched_ret` |

The scope stack is `local_ids`, an `arr_mini_hashset` pushed by
`start_of_block()` and popped by `end_of_block()`, both driven purely by
`SOB`/`EOB` — that is, **by indentation**. `stmt_label()` never touches it.
So a label region is not a scope in any sense today; it is a flat marker.

This matters for the whole assessment: **paragraph scope is not a
formalisation of existing behaviour, it is a new lifetime rule.**

---

### 2. the taxonomy needs one correction

Line scope is not a third kind. Grammar §2 already says a block may be a
one-liner "when the body follows its opening token on the same line (after
`->` or `::`)". `X :: 0` is the one-liner spelling of the initializing
block, not a separate construct — and the one-liner path
(`anonymous_bcond_block`) accepts only ret/expr/call, so it can never hold a
declaration.

Its only observable property is that the declared name is **write-once by
construction**: the initializing block closes at the newline, so under the
"modify only in the initializing block" rule `X :: 0` is immutable
immediately. That is worth stating explicitly, because it is the common case
and it is stronger than "immutable outside declaration scope" sounds.

Suggested wording: **one scope kind (block), two spellings (one-liner,
indented).** Paragraph, if it lands, is the genuine second kind — because it
is the only one not delimited by indentation.

---

### 3. is the paragraph scope useful?

Partly, but less than it looks, because **the flatness it advertises already
exists.**

`tests/label_test.al` already writes a whole-body loop with no indentation
and a named exit:

```
counted: => i32
    [Count] :: 0 =[]
    loop:
    [Count] + 1 =[Count]
    [Count] >= 3 loop.break->
    loop->
    ret 0
    loop.break:

    ret [Count]
```

Labels are already indent-free, and `loop.break:` is already a legal label
name that `loop.break->` already branches to. Both motivations in the
proposal — "reduce scope indents" and "a function whose whole body is a
loop" — are satisfied **today, with no new feature.**

So the marginal content of paragraph scope is exactly one thing: *ending the
lifetime of names declared inside the region.* That is real (it is not
enforced today), but it is a much narrower claim than the motivation
suggests, and it is worth deciding whether it alone pays for a second scope
mechanism.

There is also one thing the paragraph has that block scope does not: **a
nameable exit.** A block scope's generated label is `lbb<N>`, an internal
name no source can branch to, so you cannot break out of a block scope by
name. If the taxonomy says every block scope has a break label, it should
also say what that label is called — otherwise "block scopes have break
labels" is not a property a programmer can use, and the paragraph is the
only scope with a reachable exit. That asymmetry is arguably the real
argument for the feature, and the rules should make it the argument.

---

### 4. is the paragraph scope error-prone?

Yes, in six distinct ways. Four are verified against the current compiler.

**4.1 The scope extent is invisible in the layout.** Every other scope in
allang is delimited by indentation, so its extent is legible at a glance.
A paragraph's extent can only be found by scanning forward for a matching
label that may be screens away. This is the core objection: the feature's
selling point (no indentation) is exactly what removes the visual cue for
the thing it introduces (a lifetime boundary). A reader looking at a
declaration in flat code cannot tell which region owns it.

**4.2 A typo'd terminator fails silently.** Verified:

```
loop:
X :: i32{7}
loop.brk:        // typo
```
compiles clean, exit 0. The rule says the break label is mandatory, but
nothing can detect its absence — see 4.3.

**4.3 "Mandatory break label" is not checkable in a single pass.** At `foo:`
the parser must decide whether to push a scope, and the answer lives at
`foo.break:`, arbitrarily far ahead. The two escapes both cost something:

- *Push at every `foo:`.* Then plain labels open scopes that never close.
  This breaks existing code: `after_ret` uses `skip:` and `tail_value` uses
  `done:`, neither with a `.break`. `tests/label_test.al` would stop
  compiling.
- *Mark the opener syntactically.* Works in one pass, but gives up the
  `foo:` spelling the proposal is built around.

This is the load-bearing problem. It is not a diagnostics gap; it is a
consequence of `foo:` being lexically identical to a plain label.

**4.4 A dangling terminator fails silently.** Verified: `orphan.break:` with
no `orphan:` anywhere compiles clean.

**4.5 Duplicate paragraph names inside one function are caught late, or not
at all.** Verified — two `loop:`/`loop.break:` regions in one function:

- asm backend: `alc` reports nothing; the *assembler* rejects it with
  `symbol '.pa.loop' is already defined`.
- bin backend: accepted silently, no error at all.

Label names are per-function global (`emit_label` prefixes with
`context->name`), so paragraph names must be unique per function. Given that
`loop` is the obvious name and the feature is pitched at loops, collisions
are the expected case, and the failure mode is an assembler error naming a
mangled symbol with no source line — or silently wrong code.

**4.6 `.break` is a legal ordinary label name.** `foo.break:` parses today as
a plain label because `.` is not a token separator; `label_meta()` just
strips the trailing `:`. Making the suffix structural means any existing
label that happens to end in `.break` silently becomes a scope terminator.
The mangling collides too: `.pa.loop.break` is produced both by a paragraph
named `loop` and by a plain label literally named `loop.break`.

**One thing paragraphs do *not* introduce.** Jumping into the middle of a
scope is unchecked, but that is pre-existing and general — verified, a
`deep:` label inside an indented block can be reached by `deep->` from
outside the block, entering a scope past its declarations with no
complaint. Paragraphs do not create this hole. They do *widen* it, since
flat label-driven control flow is precisely where paragraph bodies live.

---

### 5. is the paragraph scope orthogonal?

No, on four counts. The first is structural and the others are namespace
pressure.

**5.1 It does not nest with indentation — it interleaves.** Verified, this
compiles today:

```
probe: I i32 => i32
    I is 1 ->
        loop:
        X :: i32{7}
        X is 7 loop.break->
    loop.break:
    ret 3
```

The paragraph opens inside an indented block and closes outside it. Two
scope-delimiting mechanisms whose regions can cross rather than nest do not
form a tree, and `local_ids` is a strict LIFO stack — at `loop.break:` the
scope to pop is not on top. Any implementation must either reject crossing
(a new rule, needing a new check) or abandon the stack. **This is the
strongest argument against the feature as specified**, and the rules are
currently silent on it.

**5.2 `lower_name:` would mean three things.** Function (with `=>`), plain
label, paragraph opener — with the third distinguished only by a token that
appears later in the file.

**5.3 It overlaps `>>` / `<<`.** That is already a forward-jump-with-merge
construct, and it is already indent-scoped (`deferred_unnamed_br` is pushed
and popped by `start_of_block`/`end_of_block`). `symbols.md` already flags
`>>` as "may be deprecated in future". Adding a second unnamed-region
mechanism before resolving the first one's fate is duplication.

**5.4 Undefined interaction with block values.** Grammar §2: "the last line
of a block yields the block's value". Does a paragraph have a value? Can
`X ::` take a paragraph body? Unanswered, and it has to be answered for the
feature to compose with `::`.

---

### 6. ending a paragraph with `:foo`

The slot is free — verified, `:foo` is currently unused syntax and errors
cleanly (`unexpected token :loop`), so nothing is displaced. It also reads
well against `symbols.md`, where `:` already means "right to left": `foo:`
opens name→body, `:foo` closes body→name, and the pair brackets visually.
It sidesteps 4.6 entirely, since it does not borrow the `.` member
namespace.

**But it loses the nameable exit, which is the paragraph's best property.**
`foo.break:` is simultaneously the terminator *and* a branch target —
`loop.break->` is how the existing loop idiom exits. `:foo` names nothing
you can branch to, so you would still need `foo.break->` for the exit, and
the construct ends up with three spellings for one region: `foo:` to open,
`:foo` to close, `foo.break->` to leave. That is worse than what it
replaces.

It also does not help with 4.3 — you still cannot tell at `foo:` whether a
`:foo` is coming — so it fixes the cosmetic problem and leaves the
load-bearing one.

Minor: a line-initial `:foo` is easy to misread as a continuation, and `:`
is already carrying the compound `:=` / `:+` operators.

**Recommendation: keep `foo.break:`.** If the terminator must change,
change the *opener* instead — that is the position where a single-pass
parser has to make its decision.

---

### 7. defects in the rules as written

- **D1.** "block scope: break label is automatically generated at the end of
  the scope" — only `cond ->` blocks do. `::` blocks emit none; a function
  emits `.ret` only when `has_branched_ret`. And the generated name
  (`lbb<N>`) is unnameable, so the property is not usable from source.
  Either narrow the rule to conditional blocks or specify the label's name.
- **D2.** Line scope is the one-liner spelling of a block, not a third kind
  (§2 above).
- **D3.** "an object can only be modified in its initializing block"
  contradicts the stack-object idiom in the test suite: `[Count] :: 0 =[]`
  followed by `[Count] + 1 =[Count]` mutates outside the initializing block,
  in `tests/label_test.al`. Either stack objects are exempt from the rule,
  or every loop counter needs `&` — and `&` is not implemented. Decide and
  write it down; this is the rule most likely to break existing code.
- **D4.** "outer scopes cannot access inner scope's variable" is in tension
  with §2's "the last line of a block yields the block's value". The
  resolution is that the *value* escapes and the *name* does not — say so,
  because it is the one sanctioned exception.
- **D5.** The paragraph rules do not state: whether paragraphs nest; whether
  they may cross an indent boundary (§5.1); whether names must be unique per
  function (§4.5); whether `foo->` or `foo.break->` are legal from outside
  the region; whether a paragraph has a value; what an unterminated
  paragraph at end-of-function means.
- **D6.** Reserving the `.break` suffix collides with the member namespace
  and with the existing label mangling (§4.6).
- **D7.** The immutability rule is stated as "immutable outside declaration
  scope", but for the one-liner form that means write-once immediately.
  Worth saying directly.
- **D8.** Nothing says whether a paragraph terminator is required to be at
  the same indentation as its opener. Today indentation of a label is not
  checked at all.

---

### 8. recommendation

The lifetime rule the paragraph would add is worth having; the
label-delimited *spelling* of it is what causes 4.3 and 5.1. Two steps,
in order:

**Step 1 — make the existing convention checkable, with no semantic
change.** Keep labels flat exactly as they are. Add one check: a
`foo.break` label requires a preceding `foo` label in the same function, and
warn on `foo:` opened without `foo.break:` where the function contains any
`foo.break->`. This catches 4.2 and 4.4, costs nothing in the single pass,
breaks no existing code, and gives real data on whether the pairing is used
enough to justify step 2.

**Step 2 — only if step 1 shows the idiom carries its weight.** Introduce
the scope with a *marked opener*, so the push decision is local. Then define
the answers to D5 up front, and reject a paragraph that crosses an indent
boundary rather than leaving 5.1 to the implementation.

**Prerequisite for either step: fix label diagnostics first.** Today an
unresolved label surfaces from the backend as
`unresolved label 'probe.loop.break.0'` — a mangled name with no file, line,
or column. If paragraphs land, that becomes the dominant error path, and
every mistake in §4 reports this way. Also worth a separate bug: the bin
backend silently accepted duplicate labels that the asm backend and the
assembler both rejected (§4.5).

---

### 9. note on naming

`proposals/paragraphs.md` already exists and describes a completely
different feature — TOML-table-style member prefixing
(`physical .` / `.color = "orange"`). Two unrelated features named
"paragraph" will not survive contact with the spec. Rename one before
either ships; "paragraph scope" here is the one with a claim on the word
*scope*, so the other may want to be "table" or "prefix block".
