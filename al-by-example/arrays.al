// https://gobyexample.com/arrays

std.
#alias .io.print
#alias .slice
#alias .fat

a :: i32 5 { 0 } // not technically array, just five registers named a.0 ~ a.4
// expands to
// a.0, a.1, a.2, a.3, a.4
"emp: ", a print=>

100 =a.4    // mov value 100 to register a.4
"set: ", a print=>
"get: ", a.4 print=>

[b]= :: i32 5 { 1, 2, 3, 4, 5 } // an array on stack
// it loads numbers to five registers(it does not mean to be a discrete 5 registers, cpu will rename registers anyways) and stores on the stack.
// now b is an stack offset, pointing to the start of the array.
[b.0], [b.1], [b.2] // you could access like this. it is bound checked in compile time.
[b, i32 3] // also like this, but it is not bound checked. it loads b + sizeof i32 * 3, which will be calculated in compile time.

[b]= :: i32 { 1, 2, 3, 4, 5 }

[b]= :: { 100, 3: 400, 500 }

b :: { 1, 2, 3, 4, 5 } @std.arr.#.new   // new array on heap, register b holding address to it.

two_d :: 2 { i32 3 { } }
i :: 0, i < 2, i += 1 @for
    j :: 0, j < 3, j += 1 @for
        i + j =[two_d, i, j]

"two_d: ", two_d print=>

two_d :: {
    { 1, 2, 3 },
    { 4, 5, 6 },
}
"two_d: ", two_d print=>

// https://doc.rust-lang.org/stable/rust-by-example/primitives/array.html

xs :: i32 5 { 1, 2, 3, 4, 5 }
[ys]= :: i32 500 { }         // now you allocate it on stack so you can have 500 elements
// ys :: i32 500 { }        // this is a compile error as you do not have 500 registers to hold ys.0 through ys.499.
// i32 500 ys ::            // if you do not want zero fill, do this. it only declares it.
// which is shorthand for

zs :: #sizeof i32 500 std.heap.alloc=>  // you can allocate on the heap also
i32 500 { } =[zs]    // you'll have to zerofill after allocation.

zs :: #sizeof i32 500 std.heap.alloc.z=>  // you could use this

ws :: 500 @std.arr.i32.new       // or just take a shortcut!

"First element of the array: ", xs.0 print=>
"Second element of the array: ", [ys.1] print=> // you have to load from stack, so you have these [] around it.

"Number of elements in array: ", #countof xs print=>  // this is done in compile time.
"Number of elements in array: ", #countof ys print=>  // this is done in compile time.
// "Number of elements in array: ", #countof zs print=>  // this is going to be 1. it's just a scalar address
// "Number of elements in array: ", #countof ws print=>  // countof ws is 2! address to data and length!
"Number of elements in array: ", ws.len print=>  // you can check the length dynamically by using arr!

"Array occupies ", #sizeof xs, " bytes" print=> // done in compile time
// "Array occupies ", #sizeof ws, " bytes" print=> // sizeof ws is 16! addr(8) + i32(4) + padding


// https://doc.rust-lang.org/stable/rust-by-example/primitives/array.html

analyze_slice: (i32 @sliced s)
    "First element of the slice: ", [s.data].0] print=>
    "The slice has ", s.len, "elements" print=>

// xs @slice analize_slice=>   // you cannot slice registers...
ys, #sizeof ys @fat analize_slice=>   // slice on stack array
zs, 1..4 @slice analize_slice=>  // ze[2..99]
ws @slice analize_slice=>   // this works because it expands to ws.data, ws.len!

empty_array :: u32 0 { }

std.arr.#T:
    @inline (std.arr.#T arr) at: (i32 i =>#T val, err error?)
        we, i, at=>

    at: (std.arr.#T self, i32 index =>@T val, err error?)
        index >= self.len ->
            null, err ret
        ->
            [self.data, @T index] %
            % , ok ret

    at: (std.arr.#T self, i32 index =>option T)
        index >= self.len ->
            null ret
        ->
            [self.data, @T index] %
            some, % ret

#enum err {
    err, ok
}

@enum union option T {
    null,
    T some
}

@generic option.#T: (@type T)
    @enum union {
        none,
        @T some
    }

i :: 0; ws.len + 1; @for.n
    e, value :: ws[i] @at
    e ? ..some -> i, ": " , value
    -> "Slow down! ", i, " is too far!"
    print=>

i :: 0; ws.len + 1; @for.n
    value, e :: ws[i] @at
    e ? ..ok -> i, ": " , value
    -> "Slow down! ", i, " is too far!"
    print=>

i :: 0..ws.len + 1; @for.n
    t :: i, ws at=>
    t.error err->
        "Slow down! ", i, " is too far!" print=> ret
    i, ": " , t.value print=>
