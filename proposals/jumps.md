## jump safety

Assessment of the two proposed jump rules. This is about the design, not the
current compiler — none of this is implemented.

**The rules under assessment**

1. Backward jump allowed only if the label's name starts with `loop`, and you
   are in that label.
2. Forward jump allowed only if the target is an `x.break` label, and you are
   in label `x`.

where "you are in the label" means the previous label declared.

---

### 0. what the rules have to buy — and why the obvious property is not enough

The tempting formulation is:

> a jump may leave scopes, but may never enter one.

**That is necessary but not sufficient**, and C is the proof. C's own goto
constraint (C11 §6.8.6.1) is exactly this rule, narrowed to one case: a goto
may not jump from outside the scope of an identifier of variably modified type
to inside it. C stops there and leaves the rest as undefined behaviour. Three
things escape the rule.

**0.1 Scope must mean the scope of a *name*, not the scope of a *block*.**

```c
{
    goto skip;
    int x = 5;
skip:
    printf("%d", x);   // same block throughout; no block was entered
}
```

Read block-granularly the jump is legal — one block, start to finish, nothing
entered. Read name-granularly it is not: `x`'s scope begins at its declarator,
the label sits inside that scope, and the jump enters it from outside. Note
this is precisely how C words its own VM-type rule — "the scope of an
identifier", not "a block". The block reading is the one that lets a goto skip
a declaration and land where the name is still live.

For allang the name-granular reading is worth a lot more than it is worth in C,
because `X :: value` fuses declaration and initialization. A name's scope opens
exactly where its value is established, so "never enter a name's scope" and
"never skip an initialization" collapse into the same rule. **This gives a
second, cheaper way to close Hole 2**: either make the region a scope so the
skipped name is dead at `x.break:`, or read scope name-granularly so the jump
past `B :: 2` is illegal in the first place. The first is more permissive and
probably the better choice — it keeps the jump legal — but either closes it.

**0.2 In C the rule cannot close the hole at all, because declaration is
separate from initialization.**

```c
{
    int x;             // in scope, no value
    goto skip;
    x = 5;
skip:
    printf("%d", x);   // indeterminate — no scope entered, name-granular or not
}
```

`x`'s scope begins at `int x;`, and the jump neither enters nor leaves it.
Nothing about scopes can see this; it needs definite-assignment analysis, a
dataflow problem, which is what Java and C# do and what C declined to do. So
for C the answer is a flat no. allang escapes this only because it has no
declaration-without-value form — which is a reason to keep it that way. **If
allang ever adds one, the scope rule stops being sufficient and this design
needs dataflow.**

**0.3 "Leaving a scope" is free in C only because C has no destructors.**

C leaves a scope by adjusting a stack pointer, so branching out of three scopes
costs nothing. `toc.md` commits allang to "tying lifetime of a resource to
scope" and reference-counted pointers. The moment a scope exit has a *action* —
a refcount decrement, a release — "may leave scopes" stops being a permission
and becomes an **obligation**: the jump has to run every scope exit between the
site and the target, not branch over them. A `->` that leaves three scopes must
unwind three, in order. This is invisible in the rules as written, and it is the
part most likely to be discovered late, because it only breaks once resources
are attached to scopes.

C's remaining loose ends are worth knowing but do not carry over: a backward
jump past a VLA declaration re-executes it without leaving the scope, so the
storage is not reclaimed; and `switch` gets to jump into the middle of a block
by construction, which is what Duff's device exploits.

**What the rules must actually enforce**, then:

> a jump may leave scopes and must unwind each one it leaves; it may never
> enter the scope of a name, where a name's scope begins at its declaration.

Measured against *that*, the two rules get the shape right. Rule 1 is
especially strong: it gives each loop region **exactly one backward target, at
the region's head**, which is the single most important restriction — back
edges re-run the region from the top rather than landing mid-region.

Restricting targets to region endpoints also buys something the scope rule
alone does not: **flow-sensitive facts survive.** A `!` check establishes a
fact about a value that later code depends on, and a fact is not a scope, so
scope rules cannot protect it. Because the only landing points are `x:` and
`x.break:`, no jump can land *between* a check and the use it guards. That is a
real and somewhat lucky benefit of the design, and a reason not to loosen the
target restriction later.

The holes below are all in what the rules leave unsaid.

---

### Hole 1 — nothing ties a label's position to the jump's position

This is the one that breaks the goal outright. Both rules constrain *which
label* you may name; neither constrains *where that label sits*. Labels can be
declared at any indentation, and indentation is what delimits scopes.

**1a — forward, entering a block (rule 2):**

```
loop:
    Cond ->
        Y :: compute =>
        loop.break:          // break label declared inside the if-block
        Z :: Y + 1
    Flag is 1 loop.break->   // forward ✓  is a .break ✓  in loop ✓
```

Every condition of rule 2 is met. The jump enters the `Cond ->` block, skips
`Y :: compute =>`, and `Z :: Y + 1` reads an uninitialized `Y`.

**1b — backward, entering a block (rule 1):**

```
Cond ->
    loop:
    Y :: compute =>
    ...
Something loop->             // backward ✓  starts with "loop" ✓  in loop ✓
```

The previous declared label is still `loop`, so rule 1 permits it, and the jump
enters the `Cond ->` block from outside.

**1c — same indentation is not the same scope.** A containment check cannot be
a numeric indent comparison:

```
A ->
    loop:
    ...
B ->
    loop->                   // same indent, different block
```

Both labels sit at indent 4, but the jump leaves B's scope and enters A's.

**Fix.** Add the missing rule: *the target label must be declared in a block
that encloses, or is, the jump's block.* The cheapest way to get this for free
is to also require **`x:` and `x.break:` to be declared in the same block** —
then a region's extent is visible in the layout, and any jump from inside the
region to either of its two endpoints is automatically outward-only.

---

### Hole 2 — the rules do not make the region a scope, so rule 2 can still skip an initialization

Rule 2 lets you jump forward over declarations:

```
loop:
A :: 1
Cond loop.break->
B :: 2
loop.break:
C :: B + 1                   // B never initialized on the jumping path
```

The jump is legal by rule 2, and `B` is read uninitialized. Rules 1 and 2 close
this **only if names declared between `x:` and `x.break:` are out of scope at
and after `x.break:`** — i.e. only if the label region really is a scope, the
paragraph-scope lifetime rule from `scopes.md`.

Worth stating plainly, because it is a dependency rather than a hole in the
rules themselves: **this jump design is not sound on its own. It requires the
region to be a scope.** If paragraph scope is not adopted, rule 2 needs a
different guard — something like "a forward jump may not skip a declaration
that is live at the target", which is far more work to specify and to check.

---

### Hole 3 — function labels are jump targets

A function is `lower_name:` with a signature. Nothing in the rules excludes it.
So in a function named `looper:`, the body's first region is the function
itself, and:

```
looper: X i32 => i32
    ...
    looper->                 // backward ✓  starts with "loop" ✓  in looper ✓
```

is a backward jump to the function's own entry that bypasses the call — no new
frame, parameters never re-supplied, return address untouched. That is exactly
the undefined state the design is trying to prevent, and it arrives through a
label the programmer did not think of as a label.

**Fix.** Restrict both rules to non-function labels in the current function.
Function entries are reachable only by `=>`.

---

### Hole 4 — three other constructs name a branch target, and the rules only mention jumps

Per grammar §7.2–7.3 the language has four ways to name a target, not one:

| form | covered by the rules? |
| --- | --- |
| `jump ::= lower_name "->"` | yes |
| `cond_branch ::= cond_expr lower_name "->"` | presumably, but unstated |
| `check_action ::= lower_name "->"` (as in `V ! done->`) | **no** |
| `merge_branch ::= ">>" / "<<"` | **no** |

The check-op form is a real goto: `V ! done->` branches to `done` on failure.
Two consequences worth deciding deliberately rather than by omission:

- Applied consistently, the rules **outlaw every check-failure label that is not
  a break label** — you would have to write `V ! loop.break->`. That is arguably
  a good outcome and quite elegant, but it is a language change hiding inside a
  jump rule, so it should be an explicit decision.
- The check-failure target can be established at one site and used at another
  (a named conditional sets the failure target that a later `!` reuses). The
  "are you in x" test therefore has to be applied **where the branch is
  emitted**, not where the label name was written. Checking it at the name's
  site is a bypass.

`>>` / `<<` is a *forward* jump to an unnamed merge point, so rule 2 forbids it
outright. It is in fact safe — `<<` pairs with `>>` inside the same block, so it
can only leave scopes — but the rules must either exempt it explicitly as a
compiler-verified structured jump or drop it. `symbols.md` already flags `>>` as
a deprecation candidate; this design is a reason to settle that.

---

### Hole 5 — "the previous label declared" never restores, which forbids nested loops

Because a label region never closes under this definition, declaring any label
permanently takes you out of every earlier one:

```
loop:
    ...
    loop_inner:
    ...
    loop_inner->             // legal
    ...
    loop->                   // ILLEGAL: previous declared label is loop_inner
    loop.break->             // ILLEGAL for the same reason
```

So there is no outer-`continue` and no multi-level `break`. This is an
expressiveness hole rather than a safety hole, but it is the kind that
manufactures unsafety: people work around it with flag variables and duplicated
exit tests, which is where real bugs live.

**Fix, and it costs nothing.** Define "you are in x" as **x is one of the
enclosing open regions** (a stack), not "the textually previous label". Once
regions are required to nest (Hole 1's fix), the enclosing set is well defined,
and jumping to an enclosing region's head or break still only *leaves* scopes —
so soundness is unchanged while nested loops become expressible:

```
loop:
    inner:
    Cond loop.break->        // enclosing region ✓ — exits inner, then loop
    inner.break:
loop.break:
```

This is strictly better than the textual definition on both axes. It is the
single change I would most want in these rules.

---

### Hole 6 — `loop.break` starts with `loop`

Rule 1 is a string-prefix test, and the prefix matches the break label too. So
once `loop.break:` is declared you are "in `loop.break`", whose name starts with
`loop`, and a backward jump to it is legal:

```
loop:
...
loop.break:
X :: 5
loop.break->                 // backward ✓  starts with "loop" ✓  in it ✓
```

The break target becomes a second loop head. On its own that is not memory-
unsafe, but it means the rules cannot distinguish "loop head" from "exit of a
loop", which is precisely the distinction they exist to enforce — and combined
with Hole 1 it reopens the entering-a-block case at a label the reader reads as
an exit.

The prefix test also catches `loop_helper:`, `looper:`, `loopback:` — a naming
convention doing structural work. **Fix:** at minimum exclude names containing
`.break` from rule 1; better, mark loop regions structurally rather than by
spelling, so the property is checkable instead of conventional.

---

### Hole 7 — well-formedness is unstated

Rule 2 assumes `x.break:` exists and is unique. Nothing says so. Needed:

- `x.break:` must exist for every region `x` that is jumped out of, and must
  **follow** `x:` (otherwise the "forward" jump is backward and rule 2 silently
  does not apply).
- Labels must be unique per function — otherwise `loop.break->` has two targets.
  Note this collides with rule 1's naming pressure: every loop wants to be
  called `loop`, and two loops in one function cannot be.
- If `@` macros can synthesize label names, the rules must be checked after
  expansion.

---

### Hole 8 — the back edge re-initializes immutable objects

`X :: 1` inside a loop region is re-executed on every back edge. Under "an
object can only be modified in its initializing block", a back edge is a second
initialization of an immutable object. This is surely intended — it is how loops
work — but as written the immutability rule and rule 1 contradict each other.
State it: **re-entering a region's head re-enters the initializing blocks
inside it, and re-initialization on a back edge is not a modification.**

---

### Summary — the rule set that actually closes the property

Rules 1 and 2, plus what is missing:

- **R0** targets are non-function labels in the current function *(Hole 3)*
- **R1** backward: target is the head of an enclosing loop region — one backward
  target per region *(as proposed, with Hole 5's definition of "enclosing")*
- **R2** forward: target is `x.break` for an enclosing region x *(as proposed)*
- **R3** the target label's block must enclose or equal the jump's block; `x:`
  and `x.break:` must sit in the same block *(Hole 1)*
- **R4** names declared between `x:` and `x.break:` are dead at `x.break:`
  *(Hole 2 — this is the paragraph-scope rule; R2 is unsound without it)*. The
  alternative is §0.1's name-granular reading, which forbids the jump instead
  of killing the name; R4 is the better of the two because it keeps the jump
  legal.
- **R8** a jump must run every scope exit between the site and the target, in
  order, not branch over them *(§0.3 — only bites once scopes own resources,
  but it is a codegen obligation, not a check)*
- **R5** R0–R3 apply to every construct that names a target — jumps, named
  conditionals, and `!` check actions — checked at the point the branch is
  emitted *(Hole 4)*
- **R6** `>>` / `<<` either explicitly exempt as structured forward jumps, or
  removed *(Hole 4)*
- **R7** `x.break` exists, is unique, follows `x:`; labels unique per function;
  rules applied post-macro-expansion *(Hole 7)*

With R0–R8 no jump can enter the scope of a name, and every scope a jump leaves
is unwound rather than skipped, so program state stays defined. R3, R4 and R5
are the load-bearing additions: R3 because scopes are delimited by indentation
and the rules never mention it, R4 because a forward jump can otherwise skip an
initialization, and R5 because the check operator is a second goto the rules do
not currently see.

The reason this can work in allang and cannot work in C is §0.2: allang has no
declaration-without-value form, so a name's scope and its initialization begin
at the same point. That single property is what makes a scope rule sufficient
here. It is worth treating as load-bearing rather than incidental.
