```
rationale:
    remove 'addr' Notation. it is redundant because mutate (&) implies addr.
    big structs(>8 bytes) cannot be passed by value anyways.

    downside of this is that it might seem confusing at first,
    because only pass-by-values won't have to load or store to memory.

    pointers are immutable anyways, so '&addr i32' is illeagal by definition.
    only addr &i32 is possible, which makes it unnecessary to designate where the '&' annotation goes
```

    foo .
        3,
        [Arr * I] ! ret 0
        [Arr . I]
        . =>

kaz: Number i32 =>  // value
    Number + 3

kaz2: Number &i32 => // ptr
    [&Number] := 5

kaz2: Number out &i32 => // ptr
    [&Number] := 5

kaz2: Number &&i32 => // ptr to ptr
    [[&Number]] := 5

raz: Struct big_struct => // always ptr
    [Struct.X] + [Struct.Y]
    [Struct.X..Y] +; ret

raz2: Struct &big_struct =>
    [&Struct.X..Y] := 5, 3


kaz 3 => // ok
kaz2 *3 => // not ok, 3 is constant

Num :: 3
NumStack :: 3 =[]

kaz2 *Num => // not ok, has to be ptr
kaz2 *NumStack =>

Struct :: big_struct{.. 0}
StructStack :: big_struct{.. 0} =[]

raz Struct => // not ok, ptr expected
raz Struct =[] => // ok, copy temporarily and pass ptr
raz StructStack => // ok

raz2 *Struct => // not ok, ptr expected
raz2 *(Struct =[]) => // not ok
raz2 *StructStack => // ok

