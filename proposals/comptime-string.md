# comptime string?

should we introduce a concept of comptime string?

`grammar.md` 1 already says numeric literals have the internal type *comptime
int* until assigned or cast. the string rule is the same rule:

> a string literal of N characters has type *comptime N*u8* until assigned or
> cast.

everything else people expect from "a string literal can be passed where a
pointer is expected" is the array->pointer conversion in `pointers.md`, which
is needed for `5*i32{.. 0}` anyway. it is not string-specific and is not
repeated here.

## the two rules that are string-specific

1. a string literal of N characters has type comptime `N*u8`. it is emitted
   NUL terminated. **the NUL is part of neither the type nor the length.**

2. a string literal is read only. it never coerces to a mutable (`&`) form.
   `&String :: "Hello"` is a compile error, per `slices.md`.

that is the whole proposal. the rest of this document is consequences.

## consequences

```
X :: "Hello World"          // addr 11*u8
[Y] :: "Hello World" =[]    // 11*u8, a stack copy. rule 2 does not apply
                            //   to the copy; it is ordinary memory.
Z :: slice u8{"Hello World"}
assert.that Z.Length is 11 @

strcmp "Hello World", "Bye World" =>   // raw_ptr u8, valid c string
```

`"Hello World"` is 11 characters. an earlier draft of this proposal said 12
in every case, which is 11 + NUL, and also asserted `Z.Length is 12`. that
put the terminator inside the logical content: `print` would emit a stray
zero byte, `Z is "Hello World"` would compare 12 bytes against 11, and
concatenation would embed NULs. the terminator is storage, not content.

## the c string rule

only a whole literal is NUL terminated. a sub-range is not.

```
"Hello World" .puts =>      // ok
X.1..5 .puts =>             // NOT a c string. no terminator at index 5.
```

so coercion to `raw_ptr u8` / `addr u8` is only a valid c string for an
un-sliced literal. either the compiler rejects the sub-range case, or the
cast is made explicit so the footgun is requested rather than inferred.

## which decayed forms exist

`tests/hello.al` declares `printf: Format addr u8` and passes a literal
directly. so `addr u8` -- pointer to first element, no length -- is already
the working form, and it must stay in the list; `raw_ptr u8` is the same
coercion with unchecked subscripting on top. they are one question, separated
only by mutability, not by anything about strings.

## open questions

* element type: is `slice i8` or `slice u16` ever a valid target, or `u8`
  only?
* are identical literals deduplicated? that decides what `is` means on two
  string pointers.
* escapes and the `n` suffix (`"Hello World!"n` in the README) change N.
  is N counted before or after unescaping? after, presumably -- say it.
* heredoc strings (`""EOF`, `grammar.md` 10) presumably get the same rule.
