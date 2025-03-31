// https://gobyexample.com/maps

#alias std.io.print
#alias std.map
#alias std.str

map.
m :: str, i32 @.new
"k1", 7, m .set=>
"k2", 13, m .set=>
m .to.str=> print=>

map.
v1 :: "k1", m .get=>
"v1 : ", v1.value print.fmt=>
v2 :: "k2", m .get=>
"v2 : ", v2.value print.fmt=>

map.
"k2", m .delete=>
m .clear=>

foo :: "k2", m map.get=>  // type of foo is actually @option.i32
foo.has_value print.fmt=> // false

// same stuff!
_, prs :: "k2", m map.get=>
prs print.fmt=> // false

n :: str, i32 @map.new
{ "foo", 1 } { "bar", 2 } m map.add=>

