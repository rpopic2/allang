// https://doc.rust-lang.org/stable/rust-by-example/flow_control/match.html

number :: 13

"Tell me about "number print=>

number
    ? 1 -> "One",
    ? 2 or 3 or 5 or 7 or 11 -> "This is prime",
    ? 13 > -> 19 <= -> "A teen",
    : "Ain't special",
print=>

boolean :: true

binary ::
    boolean ? 1 : 0

binary ::
    boolean
        ? -> 1
        : -> 0

triple :: { 0, -2, 3 }
"Tell me about "triple print.fmt=>
triple.
    .x ? 0 -> _, b, c :: &.
        "First is '0', 'y' is ".y", and 'z' is ".z,
    .x ? 1 ->
        "First is '1', and the rest doesn't matter",
    .z ? 2 ->
        "last is `2` and the rest doesn't matter",
    .x ? 3 -> .x ? 4 ->
        "First is `3`, last is `4`, and the rest doesn't matter",
    .x ? 3 -> .y ne? 4 ->
        "First is `3`, or middle is `4`, and the rest doesn't matter",
    :
        "It doesn't matter what they are",
print=>

array :: { 1, -2, 6 }
array.
    .0 ? 0 -> _, second, third :: &.
        "array[0] = 0, array[1] = "second", array[2] = "third,
    .0 ? 1 -> third :: &.2
        "array[0] = 1, array[2] = "third" and array[1] was ignored",
    .0 ? -1 -> second :: &.1
        "array[0] = -1, array[1] = "second" and all the other ones were ignored",

@struct Foo {
    { u32, u32 } x
    u32 y
}

foo :: Foo { x: { 1, 2 }, y: 3 }
foo.
    .x.0 ? 1 -> { x: { _, b}, y } :: &.

optional :: { Some, 7 }
optional.
    ? { Some, i: } -> "whatever"

number :: { Some, 7 }

// if let
number.e ? Some -> number.Some %
    "Matched "%"!" print=>

// let else
get_count_item: (str s =>u64, str)
    it :: s.split_iter=>
    it.next=> %; it.next=> %2
    ? { Some, &count_str }% and { Some, &item }%2 ->>
    : panic
    >> count_str u64.from_str=>
    ? { Ok, &count } ->>
    : panic
    >> count, item

"3 chairs" get_count_item

// while let
optional :: { Some, 0 }
optional @loop
    ? { Some, &i } ->
        i ? 9 > ->
            "Greater than 9, quit!" print=>
            None =optional
        :
            "`i` is `"i"`. Try again." print=>
            { Some, i + 1 } =optional
    :   <-

optional ? { Some, &i } @while
    i ? 9 > ->
        // ...
