// https://gobyexample.com/struct-embedding

#alias std.str
#alias std.print

base:
    @struct {
        i32 num
    }

    describe: (@base b =>str)
        b.num i32.to.str=>
        "base with num", % str.append=> ret

container:
    // @struct { // you could do this
    //     @base b
    //     str str
    // }

    @struct @base { // but for behaviour like the original example...
        str str
    }


co :: container {
    num: 1,
    str: "sone name"
}

co.num i32.to.str=> %
"co={num: ", %, ", str: ", co.str print.va=>

co base.describe=>
"describe: ", % print.va=>

describer:
    #alias discribe (@base =>str)
    @struct {
        @base obj
        addr #describe describe,
    }
    describe: (@describer self)
        obj describe=>
        ret

describer d :: { co, base.describe }
d describe=>

