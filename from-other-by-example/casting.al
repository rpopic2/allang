// https://doc.rust-lang.org/stable/rust-by-example/types/cast.html

// warning! casting is done with 'to' keyword, and 'as' keyword just treats that byte as if is the type!!
// pro tip: just use 'to' if you don't know what you're doing

decimal :: f32 65.4321

// u8 integer :: decimal // compile error! no implicit conversion from/to any type!

integer :: decimal as u32    // this will treat the float as integer 1115872572! yikes! you cannot make decimal to u8 at once btw.
integer :: #i32.floor decimal // now it will convert to 65!

character :: c8 integer // good! it represents 'A' character!

character :: decimal as c8

`Casting: `decimal` -> `integer` -> `character print=>


// http://odin-lang.org/docs/overview/#type-conversion

i :: 123
f :: #f64.from i
u :: #u32.ceil f

u :: f as u32

i :: noinit =[]
