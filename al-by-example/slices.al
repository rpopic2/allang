// https://gobyexample.com/slices

hello :: "Hello World!"
slice :: std.slice { hello.6, hello.11 }
fatptr :: std.fatptr { hello.6, 6 }

slice print=> // prints out "World!"
fatptr print=> // same stuff.

s :: hello, 6, 5 @std.str.slice

slice @.at 3


std.slice:
    @struct {
        addr start, addr end
    }

std.fat:
    @struct {
        addr start, isiz len
    }


// more in arrays.al
