// https://gobyexample.com/methods

#alias println = std.print.ln

rect:
    @struct {
        i32 width, i32 height
    }

    area: (addr @rect r =>i32)
        [r.width] * [r.height] ret

    permi: (addr @rect r =>i32)
        2 * [r.width], 2 * [r.height]
        + ret
        // mov w8, 2
        // ldr w9, [x0]
        // mul w8, w8, w9
        // mov w10, 2
        // ldr w11, [x0]
        // mul w10, w10, w11
        // add w0, w8, w11
        // ret

r :: rect { width: 10, height: 5 }
r rect.area=> println.fmt=>
r rect.perim=> println.fmt=>


rect.   // using this statement lets you not type rect in front of the routine!
[r]= :: rect { width: 10, height: 5 }
rp = r
[rp] .area=> println.fmt=>  // so it looks like a method!
[rp] .perim=> println.fmt=> // note that the space befor dot is mandatory.

