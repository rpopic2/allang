// https://doc.rust-lang.org/stable/rust-by-example/primitives/tuples.html

reverse: ({ i32, bool } pair => bool, i32)
    int_param, bool_param :: pair
    bool_param, int_param ret

@struct matrix: {
    f32 4
}

long_tuple :: {
    1'u8, 2'u16, 3'u32, 4'u64,
    -1'i8, -2'i16, -3'i32, 4'i64,
    0.1'f32, 0.2'f64,
    'a', true
}

"Long tuple first value: ", long_tuple.0 @va print=>
"Long tuple second value: ", long_tuple.1 @va print=>

tuple_of_tuples :: { { 1'u8, 2'u16, 2'u32 }, { 4'u64, -1'i8 }, -2'i16 }

tuple_of_tuples @va print=>
// 1 2 2 4 -1 -2

tuple_of_tuples @va print.fmt=>
// { 0: { 0: 1, 1: 2, 2: 2 }, 1: { 0: 4, 1: -1 }, 2: -2 }

pair :: { 1, true }
pair @va print.fmt=>    // { 0: 1, 1: true }
pair reverse=> %; "The reversed pair is ", % @va print.fmt=> // { 0: true, 1: 1 }

"One element tuple: ", { 5'u32 } @va print.fmt=>
"Just an integer: ", 5'u32 @va print.fmt=>

tuple :: { 1, "hello", 4.5, true }
a, b, c, d :: tuple // this makes it copy btw
a, b, c, d @va print.fmt=>

matrix :: matrix { 1.1, 1.2, 2.1, 2.2 }
matrix print.fmt=>

// the cool stuff is...

matrix.x, matrix.y, matrix.z, matrix.w // works..!
matrix.r, matrix.g, matrix.b, matrix.a // works..!


