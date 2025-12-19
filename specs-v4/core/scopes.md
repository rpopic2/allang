# scopes

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

foo: (addr i32 Ptr=>) (Ptr is pointing) // function gets called only if Ptr is pointing to a valid address
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

