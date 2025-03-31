// https://gobyexample.com/values

#alias std.print
#alias std.str

{ "al", "lang" } print=>

2 str.from#=>
{ "1 + 1 = ", % } print=>

true && false print=>
true || false print=>
true! print=>


#comptime (true && false) print=>

law_string_literal: c8 EOF^^    // you can set EOF to whatever you want.
raw string literal! can whatever you do!
very simple isn't it?
^^EOF

one_line_raw: str << EOF raw string literal!!! EOF

law_array_literal: u16 |
    1, 2, 3, 4, 5, 6, 7
    9, 0, 10
|

// http://odin-lang.org/docs/overview/#arithmetic-operators
// unary
i :: 1
+i
-i

// binary
j :: 1

i + j
i - j
i * j
i / j
i, j @mod
i | j
i ~ j
i & j
i << j
i >> j

i +? j  // adds
i -? j  // subs
i ? j   // cmp

// comparison
i is j
i isnt j
i < j
i <= j
i > j
i >= j
i is j and p is q
i is j, or p is q

// no address of operator!

// ternary
cond ? x : y // x and y can be register values only.

// no operator precedence

// overflow
x +? 1 ov->


