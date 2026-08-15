# allang grammar

A draft of the full allang grammar, derived from the reference implementation
in `alc-v2/main.c`. Where `main.c` and the prose specs in `specs-v4/` disagree,
this document follows `main.c`. Constructs that the prose specs describe but
`main.c` does **not** implement are collected in the final section so the spec
stays honest about what the compiler actually accepts.

## Notation

This is not classic BNF. allang uses `[ ]` and `{ }` as real operators, so this
grammar never uses them as metacharacters. The meta-language is:

| meta | meaning |
|------|---------|
| `::=` | rule definition |
| `\|` | alternation |
| `( … )` | grouping |
| `x?` | zero or one `x` |
| `x*` | zero or more `x` |
| `x+` | one or more `x` |
| `"…"` | literal terminal (exact characters) |
| `NEWLINE` | end of a logical line |
| `INDENT` / `DEDENT` | indentation increase / decrease (a block boundary) |
| `« … »` | informal prose description of a terminal |

Terminals are written in `"quotes"`; nonterminals are bare `lower_snake` names.

---

## 1. Lexical structure

allang is **indentation-sensitive**. The tokenizer (`tok()` in `main.c`) is
unusual: most punctuation is *not* its own token. A token runs until it hits a
**separator**, with a few special cases.

```
separator       ::= " " | "," | ";" | "(" | ")" | NEWLINE | NUL
```

Key lexer facts (from `tok()`):

- `(` and `)` are separators that produce **no token**. They are therefore
  cosmetic: `foo: (X i32 => R i32)` and `foo: X i32 => R i32` tokenize
  identically.
- `,` and `;` are separators. `,` additionally carries meaning at the grammar
  level (see *line*). `;` marks end of line.
- `{` and `}` are emitted as **standalone single-character tokens** when they
  begin a token; otherwise they terminate the current token.
- A token is otherwise a maximal run of non-separator characters, so multi-glyph
  lexemes like `::`, `=>`, `->`, `=[`, `..`, `*i32`, `Arr.0`, `[Arr` are each a
  single token (or a token *prefix* the parser inspects character by character).

```
comment         ::= "//" «any chars up to but excluding NEWLINE»
string_literal  ::= "\"" «any chars except '"' or NEWLINE»* "\""
```

Comments run to end of line and are skipped by the lexer. A string literal is
delimited by `"` and may not span a raw newline.

### Indentation

```
indent_unit     ::= 4 spaces        // an indentation that is not a multiple of 4 is an error
```

Crossing an `indent_unit` boundary opens (`INDENT`) or closes (`DEDENT`) a
block. Blank lines and comment-only lines are ignored when computing
indentation. Tabs are not used for indentation.

### Identifier classes

allang assigns meaning by the **first character** of an identifier:

```
upper_name      ::= «identifier beginning with 'A'-'Z'»     // registers, variables, constants, params/returns
lower_name      ::= «identifier beginning with 'a'-'z' or '_'»  // functions, labels, type names
member_name     ::= upper_name | lower_name | integer        // struct field or array/tuple index
```

- **Upper-case** names denote *named registers / variables / stack objects*,
  *constants*, and parameter/return slots.
- **lower-case** (or `_`-prefixed) names denote *functions*, *labels*, and
  *type names* (the fundamental types and struct names are all lower-case).
- A leading `_` marks a **private** symbol/type (not exported on import).

### Literals

```
literal         ::= numeric_literal | char_literal | bool_literal | string_literal
numeric_literal ::= "-"? ( «decimal digits» | "0x" «hex digits» )
char_literal    ::= "'" «one character or escape» "'"
bool_literal    ::= "true" | "false"        // evaluate to 1 and 0
```

`true`/`false`/char/`-N`/`0xN`/decimal are all recognized by `lit_numeric()`.
Numeric literals have the internal type *comptime int* until assigned or cast.

---

## 2. Program structure

A source file is the implicit `main` function (its top-level statements)
followed by zero or more function definitions. There is no separate "module"
construct; every `.al` file is compiled together.

```
file            ::= main_body fn_def*
main_body       ::= line*
```

The first `function()` call consumes top-level lines as `main`; each subsequent
call consumes one function definition.

```
block           ::= INDENT line+ DEDENT
line            ::= ( statement | expr_sequence )? NEWLINE
expr_sequence   ::= expr ( "," expr )*
```

A **block** is one or more lines at a common indentation. The last line of a
block yields the block's value (moved to a return register at function tail, or
to the declared target of a `::` block). Within a line, `,` separates
independent expressions, each landing in the *next* register; comma-separated
expressions may not refer to one another.

A block may be a **one-liner**: when the body follows its opening token on the
same line (after `->` or `::`), the block ends at that line's newline.
One-liners are valid for branch and declaration blocks, never for function
bodies.

---

## 3. Directives

```
directive       ::= "#declare" signature
                  | "#no_import_all_self"
                  | "#compile_all" filename
filename        ::= «path token, '/'-separated, resolved relative to this file»
```

- `#declare` registers a symbol's signature without a body (see *signature*).
- `#no_import_all_self` opts the current file out of the implicit self-import;
  it must appear before any signature is registered.
- `#compile_all` compiles another file and imports all of its symbols
  (including private ones), then continues.

> `main.c` implements exactly these three. The prose specs also describe
> `#import`, `#import_all`, `#compile`, and `#allow_import_multiple_times`
> — see §10.

---

## 4. Definitions

### 4.1 Signatures (functions, labels, `#declare`)

```
fn_def          ::= signature NEWLINE block
signature       ::= lower_name ":" sig_params?
sig_params      ::= param* ( "=>" ret* )?
param           ::= upper_name type        // name then its type
ret             ::= upper_name type
```

A signature is a `lower_name` immediately followed by `:`. Everything up to
NEWLINE is the parameter/return list. `=>` separates parameters from returns and
marks the symbol as a *function* (callable). Without `=>`, the `lower_name:` is a
plain **label** (a branch target), not a function.

Because `(` and `)` are non-tokens, signatures are commonly parenthesized for
readability:

```
foo: X i32, Y i32 => R i32
foo: (X i32, Y i32 => R i32)        // identical
nop: =>                             // function, no params, no returns
done:                               // label, no signature
```

In the parameter/return list the parser counts each `upper_name` as one
slot and parses each `lower_name` (with declarators) as that slot's `type`.

### 4.2 Structs

```
struct_def      ::= "struct" lower_name? "{" struct_member* "}"
struct_member   ::= member_name type ","?
```

A struct is introduced by `struct`, an optional name (when omitted, the
enclosing label's name is used), and a `{ … }` body of `name type` pairs.
Field separators (`,`) are optional because `,` is a lexer separator. Padding
and layout follow declaration order.

```
struct point { X i32, Y i32 }
struct point { X i32  Y i32 }       // identical (comma optional)
```

### 4.3 Constants

```
const_def       ::= upper_name ":" const_value
const_value     ::= numeric_literal | char_literal | bool_literal | upper_name
```

A constant is an `upper_name` directly followed by `:` and a compile-time value
(`Hello: 1`). Constants are immutable and substituted at compile time. (Note the
disambiguation with §4.1: a *constant* starts upper-case, a *label/function*
starts lower-case.)

### 4.4 Variable / register / stack declarations

```
decl            ::= reg_decl | stack_decl
reg_decl        ::= upper_name "::" decl_rhs
stack_decl      ::= "[" upper_name "]" "::" decl_rhs
decl_rhs        ::= expr_sequence                 // one-liner
                  | fn_call                       // one-liner
                  | NEWLINE block                 // multi-line; block must assign/store
```

- `Name ::` declares a **named register** (immutable outside its own declaration
  block). It holds a value, a function result, or the value of a multi-line
  block.
- `[Name] ::` declares a **stack object**. Its multi-line/one-liner block must
  end in a store (`=[]`).

```
I :: 1
World :: 2
J ::
    i * 4
    bytes alloc =>        // last line is the value of J
[Arr] :: 5*i32{.. 0} =[]
```

---

## 5. Types

```
type            ::= declarator* base_type check?
declarator      ::= "addr"                     // pointer (DK_ADDR)
                  | "slice"                    // slice  (DK_SLICE)
                  | array_prefix               // array  (DK_ARRAY)
array_prefix    ::= integer "*"                // e.g. "5*"  -> array of 5
check           ::= "!" integer                // DK_CHECK: bounds/error tag
base_type       ::= fund_type | lower_name     // fundamental or struct name
fund_type       ::= "u8" | "u16" | "u32" | "u64" | "u128" | "usize"
                  | "i8" | "i16" | "i32" | "i64" | "i128" | "isize"
```

Declarators are written as prefixes. Examples: `addr u8`, `slice i32`,
`addr i32`, `5*i32` (array of five `i32`). The array length prefix and the
type name are part of one token (`5*i32`), as is the value-form `i32{…}`.

---

## 6. Expressions

```
expr            ::= aggregate_literal
                  | string_literal
                  | load
                  | regable_chain

regable_chain   ::= regable ( binary_op regable )*
```

There is **no operator precedence**; a chain is evaluated left to right, and
`,` breaks the chain into separate registers.

### 6.1 Regables (operands)

```
regable         ::= literal
                  | reg_ref
reg_ref         ::= "^"* upper_name access*
access          ::= "." member_name        // struct field or static index
                  | slice_suffix           // see §6.5
```

`^` prefixes select an outer scope (`^I`, `^^I`). `Reg.Field`, `Arr.0` (static
index), and `Slice.Length` are all `access` forms.

### 6.2 Operators

```
binary_op       ::= "+" | "-" | "*"            // arithmetic (also '*' = dynamic index/slice)
                  | "shl"                       // shift left
                  | "<" | ">" | "<=" | ">="    // comparison (signed/unsigned by operand type)
                  | "is" | "isnt"              // equality / inequality
```

`is`/`isnt` and the comparisons set a condition flag; followed by `->` they form
a conditional branch, otherwise (in a parameter position) they promote to a 0/1
boolean. Constant `is`/`isnt` comparisons are folded at compile time.

### 6.3 Loads and stores

```
load            ::= "[" load_target offset? unchecked? "]"
load_target     ::= reg_ref | "This" | «empty (current target)»
offset          ::= "*" regable                // dynamic element offset
unchecked       ::= "unchecked"                // skip the bounds check

store           ::= store_src "=[" store_target? "]"
store_src       ::= expr
store_target    ::= reg_ref | "This" | «empty (current declaration target)»

reg_assign      ::= expr "=" assign_target
assign_target   ::= "^"* upper_name | "This" | «empty (current target)»
```

`[X]` loads; `X =[Dst]` / `X =[]` store; `X =Name` / `X =This` / `X =^J` assign
to a named register. `=` moves **left to right** (source on the left). Examples:

```
[Arr.0]                 // static load
[Arr * Index]           // dynamic load (bounds checked)
[Arr * Index unchecked] // dynamic load, no check
0 =[B]                  // store 0 through pointer B
7 =[J]                  // store 7 to stack object J
3 =I                    // assign 3 to register I
5 =^J                   // assign 5 to J in the outer scope
```

### 6.4 Aggregate literals

```
aggregate_literal ::= type "{" agg_body "}"
agg_body          ::= agg_field* zero_fill?
agg_field         ::= "." member_name agg_value
agg_value         ::= regable | aggregate_literal | inferred_aggregate
inferred_aggregate ::= "{" agg_body "}"        // nested type inferred from field
zero_fill         ::= ".." "0"                 // zero all unspecified members
```

Field separators (`,`) are optional. `.. 0` zero-fills the remainder.

```
point{.X 1 .Y 2}
point{.X 1, .Y 2}                       // comma optional
5*i32{.0 0 .1 1 .2 2 .3 3 .4 4}         // array, indices as members
5*i32{.. 0}                             // zeroed array
nested_3{.A point64{.X 1 .Y 2} .. 0}    // nested, named inner type
nested_3{.A {.X 1 .Y 2} .. 0}           // nested, inferred inner type
```

### 6.5 Slices and ranges

```
slice_suffix    ::= static_range | dyn_range
static_range    ::= "." integer? ".." integer?     // attached to reg_ref, constants only
dyn_range       ::= "*" regable? ".." regable?      // dynamic, bounds checked
```

```
Arr..           // whole array
Arr..3          // begin .. index 3 (exclusive)
Arr.1..         // index 1 .. end
Arr.1..3        // index 1 .. 3
Arr * I..       // dynamic begin (checked)
Arr * ..I       // dynamic end (checked)
Arr * I..J      // dynamic begin and end (checked)
```

---

## 7. Statements

```
statement       ::= directive
                  | struct_def
                  | const_def
                  | decl
                  | ret_stmt
                  | branch_stmt
                  | label_stmt
                  | checked_stmt
                  | store
                  | reg_assign
                  | control_flow
                  | expr_sequence
```

### 7.1 Return

```
ret_stmt        ::= "ret" expr_sequence?
```

`ret` jumps to the function's return block. Its operand count must match the
function's declared return arity.

### 7.2 Branches and control flow

```
branch_stmt     ::= cond_branch | jump | merge_branch
cond_branch     ::= cond_expr "->" ( NEWLINE block | line )   // anonymous conditional
                  | cond_expr lower_name "->"                  // named conditional
cond_expr       ::= regable ( ("is"|"isnt") | ("<"|">"|"<="|">=") ) regable
jump            ::= lower_name "->"                            // unconditional jump to label
merge_branch    ::= ">>"                                       // forward branch marker
                  | "<<"                                       // matching merge point
label_stmt      ::= lower_name ":"                             // local label (no signature)
control_flow    ::= jump | fn_call
```

```
I is 0 ->
    "zero" print =>
I is 5 -> ret 0         // one-liner conditional
done->                  // unconditional jump to label 'done'
>>                      // jump forward to the matching '<<'
    Z :: 1
<<
```

### 7.3 Bounds / error check

```
checked_stmt    ::= checkable "!" ret_stmt
checkable       ::= load | dyn_range_expr | fn_call    // produces a DK_CHECK value
```

The `!` operator consumes a check result and requires a following `ret` that
fires when the check fails (out of bounds, or an error tag from a checked
return type).

```
[Arr * Index] ! ret           // bounds-checked load, return on failure
Arr * Begin.. ! ret           // bounds-checked slice
Arr * Index ! ret 3           // return value 3 on failure
```

### 7.4 Function calls

```
fn_call         ::= arg_seq? lower_name arg_seq? "=>"
arg_seq         ::= expr ( "," expr )*
```

allang calls are **postfix**: arguments precede the call and are placed in
parameter registers, then `name =>` invokes. Arguments may appear before the
name, after the name, or on a preceding line.

```
"Hello World!\n" printf =>
add 2, 3 =>
3, 4 add =>
I :: get_hi =>          // result bound to a register
```

---

## 8. Worked examples (all from `tests/`)

```
// hello.al
#declare printf: Format addr u8 => Num_Printed i32
printf "Hello World!\n" =>
ret 0
```

```
// forward.al — function defined after use
add 2, 3 =>
ret 0

add: X i32, Y i32 => A i32
    ret X + Y
```

```
// ret_oneliner.al — one-liner conditional blocks
I :: 5
I isnt 5 -> ret 1
I is 5 -> ret 0
ret 2
```

```
// nested.al — nested declaration blocks
I ::
    J ::
        3 =
    J =
0
ret 0
```

---

## 9. Reserved words and symbol glossary

Keywords recognized by `main.c`: `ret`, `struct`, `is`, `isnt`, `shl`,
`addr`, `slice`, `true`, `false`, `unchecked`, `This`, plus the fundamental
type names (`u8`…`usize`, `i8`…`isize`) and the directive words after `#`
(`declare`, `no_import_all_self`, `compile_all`).

Operator/sigil summary:

| sigil | meaning |
|-------|---------|
| `::` | named variable / register declaration |
| `:` | constant decl (upper), or label/signature (lower); right-to-left bind |
| `=>` | function call (postfix) / param-return separator in a signature |
| `->` | branch / jump to a label |
| `>>` `<<` | forward branch marker / matching merge point |
| `=` | assignment, **left to right** (source on the left) |
| `=[ … ]` | store to memory |
| `[ … ]` | load from memory |
| `{ … }` | aggregate (struct / array) literal or body |
| `.` | member / static index access (never a separate expression) |
| `..` | range (slice) |
| `*` | multiply; array length prefix; dynamic index/slice offset |
| `!` | bounds/error check operator |
| `^` | outer scope reference |
| `#` | directive |
| `@` | macro (see §10) |
| `,` | next register (also a dependency break) |
| `;` | end of line |

---

## 10. Specified but not implemented in `main.c`

The following appear in `specs-v4/` prose but are **not** accepted by the
current `main.c`. They are listed so the grammar above is not mistaken for the
full design intent.

- **Directives**: `#import`, `#import_all`, `#compile`,
  `#allow_import_multiple_times`.
- **Macros / metaprogramming**: the `@` macro system (`@inline`, `@enum`,
  macro expansion `name args @`, partial inlining).
- **Aggregates**: angle-bracket views `<…>` and array literals `*i32<…>`,
  heredoc strings (`""EOF … EOF`), unions / enum-unions, the `.is`/`memcpy`/
  `memeq` struct helpers, ternary select `cond ? a : b`, and the `? eret`
  error-on-failure form (the implementation uses `! ret` instead).
- **Booleans on conditions**: `and` / `or` combinators on `cond_expr`,
  `is pointing`, and condition-to-bool promotion as a call argument.
- **Extra data-processing operators**: `/`, `SHR`, `ROR`, `ROL`, `ADC`,
  `ADD`, `AND`, `XOR`, `ORR`, `MUL`, `DIV`, `NEG`, `NOT`, `CLZ`, `CTZ`,
  and floating-point literals.
- **Other sigils** from `spec.txt` not handled by the parser: `&` (mutable),
  `~`/`~>` (ownership), `%` (match marker), `|` (parallel), `` ` `` (format
  string), `$` (explicit register), `\` (escape/cast).
