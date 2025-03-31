@generic Error.(type T):
    @enum union {
        Ok,
        T Error,
    }

Error:
    #enum {
        Ok,
        Error,
    }

foo: (i32 i =>Error.i32?)
    i is 9 lt->
        Error.Ok
    :
        { Error.Error 34 }

bar? :: 1 foo=>
? is { Err, &errorcode } ->
    // do sth

// https://zig.guide/language-basics/errors

@generic AllocationError.(type T):
    @enum union {
        T Ok
        OutOfMemory
    }

test "error union":
    maybe_error :: AllocationError.u16 { 10 }
    no_error :: is Ok ? .Ok : 0 // catch 0

    @expect #typeof no_error is u16
    @expect no_error is 10

@generic error.(type T)
    @enum union { T Ok, Oops }

failingFunction: ( =>error.void )
    error.Oops

test "returning an error"
    failingFunction=> &
    isnt Ok ->  // catch |err|
        expect & is error.Oops
        ret

failFn: (=>error.i32)
    failingFunction=>
    isnt Ok -> ret    // try
    12 ret

test "try"
    v :: failFn=> isnt Ok ->
        expect is error.Oops
        ret
    expect v is 12

|u32 problems| :: 98

// errdefer problems += 1 // cool feature!
failFnCounter: (=>error.void)
    ~ $0 isnt Ok -> ++problems
    failingFunction=> isnt Ok ->
        ret

test "errdefer"
    failFnCounter=> isnt Ok-> &err
        expect err is error.Oops
        problems is 99
        ret

// https://zig.guide/language-basics/runtime-safety

a: 3 u8 { 1, 2, 3 }
|u8 index| :: 5

b :: [a, index u8]  // use this syntax if you don't need bound checks

b :: a[index] @at   // use this for bound check

test "unreachable"
    i32 x :: 1
    u32 y :: x is 2 ? 5 : unreachable   // should we adopt this? we could use panic for this?

asciiToUpper: (u8 x =>u8)
    'a'..'z' @in_range
        x + 'A' - 'a'
    'A'..'Z' @in_range
        x
    : unreachable

// https://www.digitalocean.com/community/tutorials/understanding-defer-in-go

"sample.txt", "This file contains some sample text." write=>
is Error -> "failed to create file" print=>

write: (str filename, str text =>error?)
    err?, file~ :: filename os.Create=>
    err isnt null ->
        // file~> .Close=> ???
        err? ret
    err?, _ :: text, file .WriteString=>
    err isnt null ->
        file~> .Close=>
        err? ret
    file~> .Close=>

write2: (str filename, str text =>error?)
    file?~ :: filename os.Create=>
    file? is Error
        file.error ret
    file :: file.Ok
    ~ file~> .Close=> %? is Error -> % ret

    text, file .WriteString=> %?
    is Error ->
        %.err ret

write3: (str filename, str text =>error!)
    file~ :: filename os.Create=>
    file is { Ok, &file } ->
        defer file~> .Close=> ret

        err, _ :: text, file .WriteString=>
        err is Error -> err ret

write4: (str filename, str text =>error?)
    file~ :: filename os.Create=>
    file isnt Ok ->
        file ret
    ~ file~> .Close=> is Error -> ret

    text, file .WriteString=>
    is Error -> ret

fileCopy: (str source, str destination =>error?)
    src~ :: source os.Open=>
    is Error -> ret
    ~ close.src=>

    dst~ :: destination os.Create=>
    is Error -> ret
    ~ close.dst=>

    n :: dst..src io.Copy=>
    is Error -> ret

    `Copied `n` bytes from `source` to `destination`\n"`

    Ok

fileCopy:
    src := os.Open(source)
    src == Error
        return src
