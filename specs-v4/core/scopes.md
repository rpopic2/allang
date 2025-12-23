## program

```
program ::= block
```

a valid program constists of at least a single block.


## block

```
var indentations ::= [\t]*
block ::= indentations^line[indentations^line | line:blank]*
```

a block consists of one or multiple lines with same indentation level. blank lines are allowed in the middle or end of a block.
statements are not considered as a block.

end of a block is where indentation of a line becomes less than the indentation of a block.

e.g.

```
foo:
    bar=>
// these blank lines are not end of a block
    baz=> // last line of a block

print "Bye"=>
```
pattern: foo:\n\tbar=>\n\n\tbaz=>\n\nprint

```
0   // end of a block.

```
pattern: 0\n\n\0

```
foo:
    bar:
        baz=>

    print "Hi"=>
```
pattern:foo:\n\tbar:\n\t\tbaz=>\n\n\tprint


## line

```
line ::= [expr][, expr:nondest]*\n
line:blank ::= \n
```
a line consists of one or multiple expressions, seperated by comma, followed by a newline character.

## expression

```
expr ::= [expr:nondest | fn_call]
expr:nondest ::= [mov | arithmetic | lit]
```

expressions evaluates to a result.
'nondest' stands for non-destructive, as function calls may destroy the contents in registers.


## scopes

point:
    struct {        // struct created in scope point
        u64 X, u64 Y, u64 Z
        .....
    }

    add: (#Self, self Other => self)    // functions created
        { Self.X + Other.X, Self.Y + Other.Y }

    equals: (addr #Self, addr self Other => bool)
        Self.X is Other.X
        and Self.Y is Other.Y

{ 1, 2, 4 }, { 2, 3 } point:equals=>   //
{ a, b }, { b, c } 

# immutability

foo:
    I :: 3
    I print=>   // free to use I

    ::          // create anonymous scope
        foo.I print=> // a scope starts with indentation
        foo.I(2)    // error! other scope's registers are immutable!

    add: *I     // creates named scope and make I mutable in this scope
        I(2)
        I print=>

# conditional functions

foo: (addr i32 Ptr=>) if (Ptr is pointing) // function gets called only if Ptr is pointing to a valid address
    ...

// the conditional check part will be inlined to the function call.

# partial inlining

```
@inline SetVisible: (#Self, #comptime bool Visible)
    #is Visible->
        true =[Self.visibility]
        Self set_dirty=>
    #isnt Visible->
        false =[Self.visibility]
```

Component. @SetVisible true@    // inlined only true path
Component. @SetVisible false@   // inlined only false path

