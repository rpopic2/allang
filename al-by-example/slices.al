// https://gobyexample.com/slices

hello :: "Hello World!"
slice :: std.slice { hello.6, hello.11 }
fatptr :: std.fatptr { hello.6, 6 }

slice print=> // prints out "World!"
fatptr print=> // same stuff.

s :: hello, 6, 5 @std.str.slice

slice @.at 3


slice T:
    @struct {
        addr T data, usiz len
    }


// more in arrays.al

// https://zig.guide/language-basics/slices

total: (slice u8 values =>usiz)
    |usize sum| :: 0
    values, v @foreach
        sum += v
    sum

test "slices"
    array: u8 { 1, 2, 3, 4, 5 }
    slice :: array, 0..3 @slice
    slice total=>
    expect is 6

test "slices 2"
    array: u8 { 1, 2, 3, 4, 5 }
    slice :: array, 0..3 @slice
    expect #typeof slice is slice u8

test "slices 3"
    array: u8 { 1, 2, 3, 4, 5 }
    slice :: array, 0.. @slice


