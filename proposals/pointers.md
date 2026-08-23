# pointer types

this document owns the pointer taxonomy and the array->pointer conversion
rules. `comptime-string.md` is downstream of it.

## conflicts to resolve first

`remove-addr.al` proposes deleting `addr` outright ("it is redundant because
mutate (&) implies addr"). this document assumes `addr` stays. both cannot
land -- decide before anything below is final.

`slices.md` asks whether slices are baked in or built in userspace. this
document assumes baked, because `Arr.1..5` needs compiler support anyway.

## the kinds

|          | carries length     | rebindable | bounds checked | writable        |
|----------|--------------------|------------|----------------|-----------------|
| addr     | no, unless array   | no         | n/a            | no              |
| slice    | yes                | no         | yes            | no, `&` opts in |
| raw_ptr  | no                 | ?          | no             | ?               |

`slice` differs from `addr` on exactly one axis: it carries a length. that is
the whole point of the split and it should stay that way.

`raw_ptr` differs on three axes at once, so it is not a point in the same
space. it is the c interop escape hatch. name it as one.

### addr

points at one object. cannot be subscripted unless it points to an array of
known length, in which case `A.1..3` and `A..` work because the length is in
the type.

### slice

an addr and a length. subscriptable, and every dynamic access is checked
against the runtime length, not the declarator amount.

### raw_ptr

a c pointer. subscriptable, unchecked. exists so `#declare`d c functions can
be called at all.

## open: what is actually mutable

the README says all variables are immutable outside their declaration scope.
if that holds, "cannot change where it's pointing to" is not a property of
`addr` or `slice` -- it is true of every binding in the language, and stating
it per-kind is noise.

that leaves `raw_ptr` "can change where it's pointing to" as the language's
only exception to its own headline rule. that is a bigger change than a
pointer kind should be allowed to make.

proposal: rebinding and writing come from the `&` axis (`slices.md`:
`&Ptr_Mutable :: Data`), not from the kind. so:

* `raw_ptr u8`  -- unchecked, not writable, not rebindable
* `&raw_ptr u8` -- unchecked, writable, rebindable

and the `?` cells in the table above are answered by the sigil, not the kind.

if that is accepted, an immutable `raw_ptr` is an `addr` with unchecked
subscripting, and the taxonomy may want two kinds plus two axes rather than
three kinds.

## the missing kind

length-carrying, rebindable, checked -- a cursor. that is `iter` / `bi_iter`
from `slices.md`. the taxonomy is deliberately under-populated; say so, so
the question stops getting re-asked.

## array to pointer conversion

these rules are what `comptime-string.md` needs, and they are needed anyway
for `5*i32{.. 0}`. write them once, here.

an array value of type `N*T` coerces, where a type is expected, to:

1. `N*T`      -- the array itself. `[Y] :: 5*i32{.. 0} =[]` copies to stack.
2. `addr N*T` -- pointer, length kept in the type.
3. `slice T`  -- pointer plus length, `Length` is N.
4. `addr T`   -- pointer to first element, length forgotten.
5. `raw_ptr T`-- same, unchecked.

rules:

* a length mismatch is an error. `"Hi"` does not coerce to `addr 12*u8`.
* 4 and 5 forget the length. that is the footgun; the README says footguns
  must be requested explicitly, so consider requiring a visible cast for 5.
* the `X ::` vs `[X] ::` declaration form picks 2 vs 1. that is the existing
  decl rule, not a new one.

## open questions

* is there a preference order when several targets are viable, or must the
  expected type be unambiguous?
* what is the type with no expected type at all (`X :: 5*i32{.. 0}` at top
  level)? currently reads as 2.
* `slice u8{Value}` in `comptime-string.md` is positional aggregate init.
  `usize{5}` and `i32{7}` already use it but `grammar.md` 6.4 only documents
  `.member value` and `.. 0`. the grammar needs the positional form written
  down before any proposal leans on it.
