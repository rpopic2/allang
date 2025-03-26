// https://gobyexample.com/values

#alias std.io.print
#alias std.str

{ "al", "lang" } print.arr=>

2 str.from#=>
{ "1 + 1 = ", % } print.arr=>

true && false print=>
true || false print=>
true! print=>


#comptime (true && false) print=>
