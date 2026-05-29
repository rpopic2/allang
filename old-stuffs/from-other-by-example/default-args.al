// http://odin-lang.org/docs/overview/#default-values

create_window: (create_window.Args args =>addr Window, Window_err)
    // do something....

    struct Args {
     str title, i32 x, i32 y, i32 width, i32 height, addr Monitor monitor =>addr Window, Window_Error,
    }
    @inline args: (=>#self)
        { x: 0, y: 0, width: 854, height: 480, monitor: not pointng }


create_window: (create_window_args args)
    // ...

create_window.
window1, err1 ::
    { "Title1" } @.args
    create_window=>

create_window.

window2, err2 ::
    { title: "Title1", width: 640, height: 360 } @.args
    create_window=>

// http://odin-lang.org/docs/overview/#rationale-behind-explicit-overloading

// we don't have one, but we have # macro for it.

to_string:
    b8: (b8 bool ->str)
    // ...
    i32: (i32 i ->str)
        // ...

true to_string.#T=>
34 to_string.#T=>


