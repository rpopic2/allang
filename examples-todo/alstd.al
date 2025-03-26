std.array:
    @struct { addr data, i32 len }

    instantiate: (@type T =>@code)
        std.array.#T:
            T @_std.array.instantiate

    @macro
    _instantiate: (@type T =>@code)
        @struct { addr T data, i32 len }

        @macro at : (isiz idx, @label err =>T @std.array self)
            index >= self.len
                ->@err
            [self.data, T index]

        @macro new : (len => @std.array.#T)
            { =>malloc : data, len }


std.str:
    c8 @_std.array.instantiate

    @macro from : (addr c8 data)
        { data, sizeof data }


std.file:
    @struct {
        ...
    }
    open:
        read: (@std.str filename => @std.file?~)
            ...

std.alloc:
    heap: (usiz len => addr)

std.io:
    print: (@std.str str)

std.opt:
    @macro (@type T) {
        b8 has_value
        @T data
    }

