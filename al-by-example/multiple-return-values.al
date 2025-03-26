// https://gobyexample.com/multiple-return-values

#alias std.print

vals: (=> i32, i32)
    3, 7 ret

a, b :: vals=>
i32 a, i32 b :: vals=>
a print.ln=>
b print.ln=>

_, c :: vals=>

// but you could also..

v :: vals=>
v print.fmt=>   //prints out "3 7"!

// all same stuffs below...
v.x print=>
v.y print=>

v.0 print=>
v.1 print=>

v.r print=>
v.g print=>

