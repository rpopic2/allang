// https://doc.rust-lang.org/stable/rust-by-example/custom_types/enum.html

// also known as tagged unions.
// it stores enum tag and the data.
// #sizeof WebEvent is i32 + i32 padding + i64 + i64

// again

#alias std.print
#alias std.fmt

@enum union WebEvent {
    PageLoad,
    PageUnload,
    KeyPress c8,
    Paste str,
    Click { x: f64, y: f64 },
}

inspect: (event @WebEvent)
    event.enum
    EWebEvent.
    ? #.PageLoad -> "page loaded" ->>
    ? #.PageUnload -> "page unloaded" ->>
    ? #.KeyPress -> "pressed ", event.KeyPress ->>
    ? #.Paste -> "pasted \"", event.Paste ->>
    ? #.Click -> "clicked at x=", %, ", y=", %, "." ->>
    << print=>

WebEvent.
pressed :: WebEvent { #.KeyPress, 'x' }
pasted :: WebEvent { #.Paste, "my text" }

pressed inspect=>
pasted inspect=>

// since it uses all values, you could do switch table, which will go much faster! simple enough!

inspect: (event @WebEvent)
    switch_table: {
    WebEvent.
        #.PageLoad: "page loaded",
        #.PageUnload: "page unloaded"
        #.KeyPress: "pressed "
        #.Paste: "pasted \"%\""
        #.Click: "clicked at x=%, y=,%."
    }
    [switch_table, event.enum]
    %, event.data print.fmt=>

#enum VeryVerboseEnumOfThingsToDoWithNumbers {
    Add,
    Subtract,
}

#alias Operations VeryVerboseEnumOfThingsToDoWithNumbers

x :: Operations.Add


VeryVerboseEnumOfThingsToDoWithNumbers:
    @enum {
        Add,
        Subtract,
    }

    run: (@self, x i32, y i32 =>i32)
        self
        ? Add -> x + y ret
        ? Subtract -> x - y ret

    // can you do switch table? yes!
    // while it might look a bit wierd, it works.
    run_switch: (@self, x i32, y i32 =>i32)
        [swich_table, self as i32]->
        switch_table: addr () {
            #Add: () x + y ret
            #Subtract: () x - y ret
        }

    // this is what it would look like in asm
    run_switch:
        adr x8, run_switch.switch_table.0
        add x8, w0, stxu 2
    run_switch.switch_table.0:
        b __LBL_0
    run_switch.switch_table.1:
        b __LBL_1
    __LBL_0:
        add w0, w1, w2
        ret
    __LBL_1:
        sub w0, w1, w2
        ret
    // you could optimise the code above if you know the code it is pointing to has same length.. but you never know how it will be assembled into. so while it involves some branches, it still works!

    // compare this in assembly. it almost works the same..!
    run:
        cbz w0, run.0
        tbnz w0, 1 run.1
    run.0:
        add w0, w1, w2
        ret
    run.1
        add w0, w1, w2
        ret


// https://doc.rust-lang.org/stable/rust-by-example/custom_types/enum/testcase_linked_list.html

List:
    @enum union {
        Cons { u32 val, addr List next },
        Nil,
    }
    @inline new: (=>List)
        Nil

    @inline prepend: (u32 elem, @self =>List)
        ptr :: #sizeof addr std.heap.alloc=> %
        ~ ptr std.heap.free=>
        self =[ptr]
        Cons, { elem, ~>% }

    len: (addr @self =>u32)
        e, v :: [self]
        ? Cons ->
            v.Cons.next len=> + 1 ret
        -> 0 ret

    stringify: (addr @self =>str)
        e, v :: [self]
        ? Cons ->
            head, tail : v.Cons
            tail stringify=> %
            head, " ,", % fmt=>
        ? ->
            "Nil"

List.
[list]= :: @List.new
1, [list] @.prepend=> =[list]
2, [list] @.prepend=> =[list]
3, [list] @.prepend=> =[list]
list .len=> %
"linked list has length: "% print=>
list .stringfy=> print=>

// https://zig.guide/language-basics/unions

#enum Tag {
    a, b, c
}

@enum union Tagged {
    u8 a, f32 b, b8 c
}

test "switch on tagged union"
    value :: Tagged { b: 1.5 }
    is { a, byte } -> [byte] + 1 =[],
    is { b, float } -> [float] * 2 =[],
    is { c, b } -> [b]! =[],

// https://zig.guide/language-basics/optionals

test "optional"
    found_index :: optional.usiz { null }
    data: i32 { 1, 2, 3, 4, 5, 6, 7, 8, 12 }
    data, i32 v, i32 i @foreach.index
        v is 10 -> i =found_index

test "orelse"
    a :: optional.f32 { null }
    fallback_value :: 0'f32
    b :: a is null ? 0 : a.value
    b :: a is value ? .value : fallback_value

test "orelse unreachable"
    a :: optional.f32 { 5'f32 }
    b :: a is value ? .value : panic
    // c :: a.?  // i don't think that we'll adopt this
    expect b is #typeof f32
    expect b is c

test "if optional payload capture"
    a :: optional.i32 { 5 }
    a isnt null ->
        a isnt value -> panic

    b :: option.i32 { 5 }
    b is (value, &v) -> 1 +=v

numbers_left :: 4'u32
eventuallyNullSequence: () option.u32 {
    numbers_left is 0 ->
        null ret
    1 -=numbers_left
    numbers_left ret
}

test "while null capture"
    sum :: u32 0
    @loop
        eventuallyNullSequence=> is (value, &v) {
            v +=sum
        }
