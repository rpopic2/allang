// # store problem

count :: noinit =[]
j :: 1

j =[count]  // generates mov j to tmp, store j to i
// tmp is not j here

0 =[count]  // generates mov 0 to tmp, store tmp to i
// tmp is 0 here

&j =[count] // moves j directly to i
=j[count]   // another syntax..?

j[=count]   // another syntax?
0[=count]   // no work
0 [=count]

=[count = j]
=[j = count]

=[j: count]
=[count: j]

[s] + 2 =[s]


// # stack allocation problem

s :: i32 0 =[]  // new object of i32
s :: i32 noinit =[] // new object of i32, no init

0, s
=[] // you cannot infer rhs inside [] from other line.

[s] + 2 =[] // ok, s is inferred.

s
[] + 2 =[]  // ok, s in inferred for load, after inferred for store

