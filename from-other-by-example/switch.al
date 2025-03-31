// https://gobyexample.com/switch

#alias std.io.print
#alias std.time

i :: 2
i
? 1 eq->> "one"
? 2 eq->> "two"
? 3 eq->> "three"
::
print=>

// you could do this but why not..

i :: 2
{ "one", "two", "three" }
[%, i addr] print=>

time.
.now=> .weekday=>
    ? #.saturday or #.sunday ->> "It's the weekend"
    _: "It's a weekday"
print=>

.time
t :: .now=>
    t .hour=> @< 12 ->> "It's before noon"
    _: "It's after noon"
print=>


what_am_i: (void i)
    #typeof i
        ? #typeof b8 ->> "I'm a bool" print=>
        ? #typeof i32 ->> "I'm an int" print=>
        _: { "Don't know type ", #typename sth } print.arr.ln=>

true what_am_i=>
1 what_am_i=>
"hey" what_am_i=>

// https://zig.guide/language-basics/switch

// is -1..1 @in_range
test "switch statement"
    |i8 x| :: 10

    is -1..1 @in_range ->
        -x,
    10 or, 100 ->
        x, 10 @div,
    : x,
    =x

    expect x is 1

test "switch expression"
    |i8 x| :: 10

    x=
        is -1..1 @in_range ->
            -x,
        10 or, 100 ->
            x, 10 @div,
        : x,

    expect x is 1
