## should be slices implemented in the userspace or baked in language?

### userspace
if implemented in userspace, user can create their pointer types.

slice type
```
struct slice T {
    Begin addr T,
    Length usize,
}
```

forward iter type
```
struct iter T {
    Current addr T,
    End addr T,
}
```

bi-directional iter type
```
struct bi_iter T {
    Current addr T,
    Begin addr T,
    End addr T,
}
```

downsides is that it makes awkward to let compiler know that the Length field is a length for Begin field.

```
Data :: 5*i32{.. 0}
Slice :: slice{.Begin Data .Length Data .len @}
[Slice * 2] ! eret // we need to check 2 agains Slice.Length
```

### baked

```
Data :: 5*i32{.. 0}
Data.0..5 // because we have a slice select index, it has to be baked
Slice :: Data.0..5 ! eret
[Slice * 2] ! eret // we need to check 2 against Slice.Length
```

btw.. we have two syntax for accessing arrays:
1. [Arr * I] // dynamic one. does multiplication at runtime.
2. [Arr.2] // static one. does multiplication at comptime.

we can do same for slices:
1. [Arr * I..J]
2. [Arr.2..3]

we also need to elaborate on the dot syntax.

* Arr.1..5 -> select all from index 1 to 5 exclusive
* Arr.1..1+5 -> select five more elements from 1
* Arr.1..5++ -> select five more elements from 1 to 5 inclusive
* Arr..5 -> select from 0 to 5
* Arr.1.. -> select all from 1
* Arr.. -> select all
* should we allow Arr.3..1? should it be sematically differenct? like iterating backwards? -> let's go empty because most languages go empty

* Arr.I..5 -> select Ith to 5. but what happens if I >= 5?
* Arr.I..I+2 -> select Ith to I+2th
* Arr.I..I+1 -> select I
* Arr..I
* Arr.I..


# mutable slices

if we blindly make all slices mutable, user can try changing slices in read-only sections and crash.

pointers:

[&Data] :: 3*i32{.. 0} =[]

Ptr :: Data // the Ptr is immutable.
&Ptr_Mutable :: Data // the Ptr i

Slice :: Data.1..3
&Slice_Mutable :: Data.1..3

String :: "Hello"
&String_Mutable :: "Hello" // compile error!

