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

law_string_literal: c8 EOF^^    // you can set EOF to whatever you want.
raw string literal! can whatever you do!
very simple isn't it?
^^EOF

one_line_raw: c8 EOF>>raw string literal!!!<<EOF

law_array_literal: u16 {
    1, 2, 3, 4, 5, 6, 7
    9, 0, 10
}
