## rationale
the following code mises optimization chance:
```
    -1 =[Tok.End] =[Tok.Start]
    0 =[Tok.Size]
```
the store is done three times here, and same =[Tok.] has been repeated three times.if Tok.End and Tok.Start is layed out to continuously, it can be done be with one stp on arm, if they span 128 bytes. or in a single mov if they span 64 bytes.

we also must define a way to memberwise copy a struct. it is not defined yet.

## syntax for copying an entire struct

let's take a look on array copying syntax:

```
Arr :: 100*i32{.. 0} =[]
Clone :: [Arr..] =[]
```

while you cannot load 100*i32 onto registers, this syntax is allowed when it is followed by store statement(s).


based on the array copy syntax, we can write:

```
Tok :: token{.. 0} =[]
Dup :: [Tok..] =[]
```

1.
```
```
## copying a part of a struct

array keeps with the slice syntax.

```
Partial :: [Arr.1..50] =[]
```

we can try matching with that for struct, however we cannot use .. syntax as that implies contiguous memory region. we still could use that, but that is fragile as member of structs can be renamed anytime.

but it does not make sense to load a part of a struct and store it, so no point of discussion here.

```
Partial :: [Tok .End .Begin] =[] // does not make sense
```

## modifying a part of a struct

modifying is simmilar to creating a new one. maybe we can make use of that syntax:

```
Make :: token{.End 0 .Start 0} =[]
token{.End -1 .Start -1} =[Make]
[Make]:= token{.End .Start -1}
```
    
after writing down the syntax, it feels like the whole struct is being clobbered.
also, it is fragile because ..0 is the only thing that makes difference. if the user forgets ..0, the whole struct won't be clobbered.
we could introduce a new syntax:

idea 1
```
Make.{
    .End .Start -1 // is it okay to initialize both here? isn't it error prone?
    .Size 0
} =[]
```

idea 2
```
[Make] := {.. Make .End -1 .Start -1}
```

## copy and modifying a part of a struct

this is useful for both user ergonomics and optimization, as you do not need to write twice.

```
// idea 1
Copy :: [Make .End .Start] =[]

// manual way
Copy ::
    [Make.End] =[.End]
    [Make.Start] =[.Start]
    0 =[..]

// like c#

Copy :: [Make..] with {.Length 0}

Copy :: {..[Make..] .Length 0}
```

original.field1 = -1
original.field2 = -1

original. {.field1 -1 .field2 -1}


