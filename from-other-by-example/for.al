// https://gobyexample.com/for

// SEE ALSO loop.al

#alias print = std.io.print.ln.#

// how to loop without macros
i :: 1
    ~ ->.
    i > 3
        <-
    i print=>
    i += 1

// with @loop macros, expands to example above
i :: 1
@loop
    i > 3 <-
    i print=>
    i += 1

// with @while macros, expands to the first example w/o macro
// same appearance as the first example in the original
i :: 1
i <= 3 @while
    i print=>
    i += 1

// with @for macros. j is not accisible outside this block.
 j :: 0; j < 3; j += 1 @for
    j print=>

// expands to
    ~ ->.       // jump here at the end of this block
    j :: 0
    j >= 3 <-   // break if condition met
    ~ j += 1    // increment at the end of this block. defer (~) statement works like a stack, so increment will happen before jumping to the start of this block
    j print=>

i, 3 @times
    { "range ", i } std.io.print.arr=>

@loop
    "loop" print=>
    <-

n, 6 @times
    n @% 2 nz<-
    n print=>

// https://doc.rust-lang.org/stable/rust-by-example/flow_control/loop.html

count :: 0

@loop
    1 +=count
    count ? 3 ->
        "three" print=>
        ->continue

    count print=>

    count ? 5 ->
        "Ok, that's enough" print=>
        <-

// nested

@loop
    "Entered the outer loop" print=>
    @loop
        "Entered the inner loop" print=>
        // <- this would only break the inner loop
        <<- // this breaks outer loop
    "This point will never be reached" print=>
"Exited the outer loop" print=>

outer: @loop
    inner: @loop
        <-outer // you can also have named loops! this makes sense if you see how @loop macro is done..

counter :: 0
result :: @loop
    counter += 1
    counter ? 10 ->
        counter * 2 <-

@assert.eq result, 20

n :: 1
n ? 101 < @while
    n @mod 15 == 0 ->
        "fizzbuzz" ->>
    n @mod 3 == 0 ->
        "fizz" ->>
    n @mod 5 == 0 ->
        "buzz" ->>
    -> n
    print=>
    1 +=n

n :: 1..101 @range
    //...

n :: 1..100 + 1 @range
    //...


names :: "Bob", "Frank", "Alvin" @std.arr.new

name..names @foreach // careful it is a dumb macro right now...
    name, "Alvin" str.eq ->
        "There is a alien among us!" ->continue
    "Hello "name print=>


// https://zig.guide/language-basics/for-loops
string: c8 { 'a', 'b', 'c' }

string, c8 character, i32 index  @foreach.index
    character =_
    index =_

string, c8 character @foreach
    character =_


// https://odin-lang.org/docs/overview/#for-statement

i :: 0; i < 10; i +: 1 @for
    i print=>

i :: 0
; i < 10; @for

i < 10 @while

@loop

// it's always exclusive
i :: 0..10 @range
    i print=>

i :: 0..9 + 1 @range
    i print=>


some_string: "hello, 세계"

some_string[c8 character] @foreach
    character print=>

some_array: 3 i32 { 1, 4, 9 }

some_array[i32 value] @foreach
    value print=>

some_dynamic_array~ :: i32 { 1, 4, 9 } @std.list.new
~ some_dynamic_array~> @std.list.delete
some_dynamic_array[i32 value] @foreach
    value print=>

some_map~ :: { string, int } {{"A", 1}, {"C", 9} , {"B", 4}} @std.map.new
~ some_map~> @std.map.delete
// dunnow how to iter map for now...

some_string[c8 character], i32 index @foreach.index
    index` `character print=>

// ..same stuffs for others

// if you wish to iterate over a ptr
some_array.
p :: .data...data + .len @range
    [p]

some_array..addr T p @loop.reference
    [p]

array :: i32 { 10, 20, 30, 40 , 50 }
array..[i32 x] @reversed.foreach
    x print=>



// https://odin-lang.org/docs/overview/#branch-statements

cond @while
    :
        is cond <-  // breaks
    <-  // breaks

loop: cond1 @while
    cond2 @while
        <-loop

cond1 @while
    cond2 @while
        >>
<<

exit:
    is true -> <-exit

cond @while
    get_foo=> is true->
        ->continue

i
is 0 ->
    foo=>
is 1 ->
    bar=>

