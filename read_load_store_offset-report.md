# Report: `read_load_store_offset`

File: `alc-v2/main.c:717-835`

```c
bool read_load_store_offset(parser_context *context, str s, reg_t *out_reg, regable *out_offset);
```

## 1. Purpose

`read_load_store_offset` parses the operand of a memory **load** (`[S]`) or
**store** (`X =[S]`) and resolves it into two outputs:

- `*out_reg` — a `reg_t` describing the base register/location that holds the
  address being dereferenced.
- `*out_offset` — a `regable` describing the element offset, either a
  compile-time `VALUE` (already scaled to bytes) or a runtime `REG`.

It returns `true` when parsing succeeded and the outputs are usable, and
`false` when the operand could not be resolved (the caller then bails out).

It is the shared front-end for the two access forms and is called from exactly
two places:

- `binary_op_store` (`main.c:947`) — the store side `X =[S]`.
- `expr_load` (`main.c:1744`) — the load side `[S]`.

The input `str s` initially points at the whole bracket token, e.g. `[Arr.3]`
or `[Arr * I]`. Note that `s` is passed **by value**, so the local
pointer arithmetic on `s.data` / `s.end` does not affect the caller.

## 2. Walkthrough

The body breaks into five phases.

### Phase 1 — Strip brackets / detect a dynamic offset (lines 718-734)

```c
regable offset_regable = {.tag = VALUE, .value = 0};
s.data += 1;                       // skip leading '['
if (s.end[-1] == ']') {            // static form  [target]
    s.end -= 1;
} else if (streq(s.end, " *")) {   // dynamic form [target * offset]
    tok(context); tok(context);
    str offset_str = cur_token->id;
    if (offset_str.end[-1] == ']')
        offset_str.end -= 1;
    offset_regable = read_regable(offset_str, cur_token);
    diagnostic_dyn_elem_access(context, &offset_regable);
} else if (s.end[-1] != ']') {
    compile_err(cur_token, "closing ']' expected\n");
}
```

The offset defaults to a compile-time `0`. `s.data += 1` drops the `[`.

- **Static form** `[target]`: the token ends in `]`, which is trimmed off,
  leaving `s` = `target`. The offset stays `0`.
- **Dynamic form** `[target * offset]`: detected because the characters
  immediately *after* the base token are `" *"` (`streq` does a prefix
  `memcmp`). Two tokens are consumed (`*` and the offset operand), the offset
  operand is read into `offset_regable` via `read_regable`, and
  `diagnostic_dyn_elem_access` emits a warning/error depending on the offset
  kind (e.g. it warns that a `VALUE` offset should use the static `[Arr.N]`
  syntax).
- **Otherwise**: a missing closing bracket is an error.

### Phase 2 — Resolve the base target (lines 736-761)

```c
regable regable_target;
if (str_eq_lit(s, "This")) {              // implicit assignment target
    target *t = arr_target_top(&context->targets);
    if (!t) { compile_err(...); return false; }
    regable_target = (regable){.tag = REG, .reg = *t->reg};
} else if (str_empty(&s)) {                // [] / =[]  -> current target
    target *targ = get_current_target_stack(context);
    if (!targ) return false;
    regable_target = (regable){.reg = *targ->reg, .tag = REG};
} else {
    regable_target = read_regable(s, cur_token);  // a named id
}
if (regable_target.tag == NONE) return false;
if (s.data[-2] != '=')                     // load (not store): operand must be set
    check_unassigned(regable_target, context);
if (regable_target.tag != REG) { compile_err(..., "register expected"); return false; }
reg_t reg = regable_target.reg;
```

Three ways to name the base:

1. `This` — the top of the target stack (`arr_target_top`).
2. empty (`[]` / `=[]`) — the current target stack entry.
3. a normal identifier — parsed by `read_regable`, which also handles member
   access, indexing, and slicing.

`s.data[-2]` peeks at the character *before* the original `[`. For a store
`X =[S]` that character is `=`; for a load `[S]` it is not. So the
`s.data[-2] != '='` test means "this is a load," in which case the source must
already be assigned (`check_unassigned`). The result must ultimately be a
`REG`.

### Phase 3 — Make sure the register actually holds an address (lines 762-785)

```c
i32 addr = dtype_tryget_addr(&reg.dtype);
if (reg.reg_type != STACK && addr <= 0) {
    member_t *first = NULL;
    ... // if reg is a struct whose first member is a pointer, reinterpret as it
    if (first) {
        reg.dtype  = first->dtype;
        reg.offset -= (i32)first->offset;
        reg.rsize  = (reg_size)dtype_size(&first->dtype);
    } else if (dtype_tryget(dtype, DK_SLICE)) {
        reg.dtype = reg.dtype;                 // no-op
        reg.rsize = (reg_size)dtype_size(dtype);
    } else {
        compile_err(cur_token, "a register containing addr is expected\n");
    }
}
```

A `STACK` location is implicitly addressable. Otherwise the register must carry
a pointer (`dtype_tryget_addr > 0`). When it does not, two recoveries are
attempted:

- The register is a **struct whose first member is itself a pointer** (and the
  struct has more than one member): reinterpret the register *as that first
  member* — adjust `dtype`, `offset`, and `rsize`. This is what lets a
  slice/pointer-wrapper struct be dereferenced.
- The register is a **slice** (`DK_SLICE`): keep the dtype and refresh `rsize`.

If neither applies, it is an error.

### Phase 4 — Compute the offset (lines 786-830)

Two cases, keyed on `offset_regable.tag`.

**Compile-time offset (`VALUE`)** — static index or the default `0`:

```c
i32 slice = dtype_tryget(&reg.dtype, DK_SLICE);
if (slice) {
    reg_t count_reg = reg;            // the length lives at offset+1
    count_reg.offset += 1;
    count_reg.rsize = sizeof (void *);
    offset_regable.value = slice;
    check_bounds(context, count_reg, slice, INCL);
}
size_t stride = reg.dtype.base ? reg.dtype.base->size : sizeof (i32); // (NULL => compiler bug)
offset_regable.value *= stride;        // scale index -> byte offset
if (reg.reg_type == STACK) {           // rebase a stack slot onto FP
    offset_regable.value += reg.offset;
    offset_regable.value = -offset_regable.value;
    reg = (reg_t){ .reg_type = FP.reg_type, .rsize = FP.rsize,
                   .dtype = {.base = reg.dtype.base} };
    dtype_push(&reg.dtype, (declarator_t){.tag = DK_ADDR, .amount = 1});
}
offset_regable.value += reg.displacement;
```

The index is scaled to a byte offset by the element `stride`. For a `STACK`
register the slot offset is folded in and negated, and `reg` is rebuilt as an
`FP`-relative address (a `DK_ADDR` declarator is pushed). Finally any
`displacement` carried on the register is added.

**Runtime offset (`REG`)** — dynamic index:

```c
if (streq(cur_token->end + 1, "unchecked")) {
    tok(context);                       // caller opted out of bounds checking
} else if (offset_regable.tag == REG) {
    declarator_t decl = dtype_top(&reg.dtype);
    if (decl.tag != DK_ARRAY && decl.tag != DK_SLICE) {
        compile_err(cur_token, "register was not an array\n");
    } else if (decl.tag == DK_SLICE) {
        reg_t count_reg = reg; count_reg.offset += 1; count_reg.rsize = sizeof(void *);
        check_bounds_reg(context, offset_regable.reg, count_reg, EXCL);
    } else {
        check_bounds(context, offset_regable.reg, decl.amount, EXCL);
    }
} else unreachable;
```

For a dynamic index, unless the source explicitly says `unchecked`, a bounds
check is emitted: against the runtime length (`count_reg` at `offset+1`) for a
slice, or against the static `decl.amount` for an array. A non-array/slice base
is an error.

### Phase 5 — Publish the results (lines 832-834)

```c
*out_offset = offset_regable;
*out_reg = reg;
return true;
```

## 3. Readability / control-flow improvements (no function extraction)

The following are local clean-ups that reduce noise and dead branches without
splitting the function up. They preserve behavior except where noted.

### 3.1 Dead / always-true conditions

- **Line 732** `else if (s.end[-1] != ']')` is *always true*: the first `if`
  already established `s.end[-1] == ']'` is false, so this `else if` can only
  run when `s.end[-1] != ']'`. Replace it with a plain `else`:

  ```c
  if (s.end[-1] == ']') {
      s.end -= 1;
  } else if (streq(s.end, " *")) {
      ...
  } else {
      compile_err(cur_token, "closing ']' expected\n");
  }
  ```

- **Line 817** `else if (offset_regable.tag == REG)` is *always true*: control
  is already inside the outer `else if (offset_regable.tag == REG)` (line 814)
  and the tag is not changed by the `unchecked` branch. Flatten it to `else`:

  ```c
  if (streq(cur_token->end + 1, "unchecked")) {
      tok(context);
  } else {
      declarator_t decl = dtype_top(&reg.dtype);
      ...
  }
  ```

### 3.2 No-op statement

- **Line 780** `reg.dtype = reg.dtype;` is a self-assignment and can be deleted.
  Only `reg.rsize` actually needs updating in that branch.

### 3.3 Make load-vs-store intent explicit

The `s.data[-2] != '='` test and the `s.data += 1` that precedes it are easy to
misread. Capturing the discriminator in a named local (computed *before*
mutating `s`) reads far better and needs no comment:

```c
const bool is_store = s.data[0] == '=';   // before the s.data += 1
s.data += 1;
...
if (!is_store)
    check_unassigned(regable_target, context);
```

This removes the negative-index pointer arithmetic and states intent directly.

### 3.4 Collapse the stride NULL-guard

Lines 796-801 use a four-line `if/else` to assign `stride`; it is already
written as a compiler-bug guard, so a conditional expression is shorter and
equivalent:

```c
if (reg.dtype.base == NULL)
    compile_err(cur_token, "compiler bug: reg type was NULL\n");
size_t stride = reg.dtype.base ? reg.dtype.base->size : sizeof (i32);
```

### 3.5 Hoist the repeated "count register" construction

The slice length register is built identically in two places (lines 789-791 and
822-824):

```c
reg_t count_reg = reg;
count_reg.offset += 1;
count_reg.rsize = sizeof (void *);
```

Since the constraint is to avoid new functions, the duplication can at least be
made obviously identical (same `sizeof (void *)` spelling — one currently uses
`sizeof (void *)` and the other `sizeof(void *)`), so a reader sees they are the
same construct.

### 3.6 Consistent error handling

The function mixes two error styles: some failures `compile_err` then
`return false` (lines 740-741, 758-759), while others `compile_err` and *fall
through* with a partially-formed `reg` (lines 783, 797, 820). The fall-through
cases rely on the surrounding compiler already being in an error state. Making
these consistent — either all return `false` or all continue — would make the
control flow easier to reason about. This is a behavioral change and should be
done deliberately.

## 4. Things worth a second look (correctness, not just style)

These are noted for the maintainer; they are *not* pure readability fixes.

- **Line 792** `offset_regable.value = slice;` overwrites the static index with
  the slice's declared length right before that value is scaled by `stride` and
  used as the byte offset. For a static slice index like `[Slice.2]` this looks
  like it discards the `2`. Worth verifying against the intended semantics.

- **Line 830** `else unreachable;` assumes `offset_regable.tag` is only ever
  `VALUE` or `REG` here. In the dynamic branch, `read_regable` (line 730) can
  return `NONE` on a bad offset operand, and that result is not re-checked after
  `diagnostic_dyn_elem_access`. A malformed dynamic offset could therefore reach
  the `unreachable`. A guard (`else return false;` or an explicit `NONE` check
  after Phase 1) would be safer.
