#declare printf: (X)
#declare foo: (X, Y => A)

printf "Hello World\n" =>

[I] ::
    4 + 9
    2 + 1 =[]
[J] ::
    [^I] =[]
    2 + 9 =[]

[J]

K ::
    2 + 33
    [P] ::
        5 =[]
    = 3 + 2

K is 3 cool->
    P :: = ^K
cool:
7 =[J]

foo 1, 2=>

I, 3, [I], [J]
"hi", "ok"
ret false, true

foo: (X, Y => A)
    ret X + Y


foo2: (X, Y => A)
    ret X + Y

