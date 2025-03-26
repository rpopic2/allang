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
