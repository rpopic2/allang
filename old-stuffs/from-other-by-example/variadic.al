// https://gobyexample.com/variadic-functions

// wip...

// variadic registers

// sums: (i32 count, i32 ... args)
sums: (i32 @va_args nums)
    total :: 0
    i32 num; nums @foreach
        total += num
    total print=>

args :: { 1, 2 }
1, 2 @va sum=>
1, 2, 3 sum=>

whatever, "hello" @va print=>

@inline va_args: (@type T) { i32 count, T ... args }
@inline va: (@reg ... r) #va_count r, r

