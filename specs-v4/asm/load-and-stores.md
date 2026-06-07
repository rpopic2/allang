## load from pointer

```
load ::=
    | '['id/nreg']'
    | '['id/nreg(.member_name)+']'
    | '['id/nreg'*'id/nreg']'
    | '['id/nreg'*'id/nreg "unchecked"']'
```

### examples

[A]             // loads from the address which register a is pointing at
0 =[B]          // stores 0 to the address which register b is pointing at

[Arr.0]         // static member / index access (bounds checked at compile time)
[Arr * Index]   // dynamic index (bounds checked)
[Arr * Index unchecked] // dynamic index, no bounds check

[Point.X]       // offsets by offset of struct member


## store to pointer

```
store ::=
    | (id/nreg | 0) ='['id/nreg']'
    | (id/nreg | 0) ='['id/nreg.id/member_name']'
```


