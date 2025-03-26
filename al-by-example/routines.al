// https://gobyexample.com/functions
// functions are called routines in al.

plus: (i32 a, i32 b => i32)
    a + b ret

plus_plus: (i32 { a, b, c } => i32)
    a + b + c ret

res :: 1, 2 plus=>
res :: 1, 2, 3 plus_plus=>

// you could inline these

@inline plus: (i32 a, i32 b => i32)
    a + b

@inline plus_plus: (i32 { a, b, c } => i32)
    a + b + c

res :: 1, 2 @plus
res :: 1, 2, 3 @plus_plus

// https://doc.rust-lang.org/stable/rust-by-example/fn.html
100 fizzbuzz_to=>

is_divisible_by: (u32 lhs, u32 rhs =>b8)
    rhs ze->
        false ret
    lhs % rhs ? ze->
        true ret
    : false ret

fizzbuzz: (u32 n)
    n, 15 is_divisible_by=> true->
        "fizzbuzz" >>
    n, 3 is_divisible_by=> true->
        "fizz" >>
    n, 5 is_divisible_by=> true->
        "buzz" >>
    : n
    print=>

fizzbuzz_to(u32 n)
    n :: 1..n + 1 @range
        n fizzbuzz=>


Point:
    @struct {
        f64 x, f64 y
    }

    @inline origin: (=>Point)
        Point { x: 0.0, y: 0.0 }

    // @inline new: (x: f64, y: f64 =>Point)
    //      Point { x: x, y: y }
    //
    // well, you could just write
    // { 123.3, 454.6 } // this instead of
    // 123.4, 564.6 @new

Rectangle:
    @struct {
        Point p1, Point p2
    }

    area: (addr @self =>f64)
        Point { x1, y2 } :: [self.p1] // & is an alias operator.
        x2, y2 :: [self.p2]          // it just gives another name instead of loading it onto register.
        x1 - x2, y1 - y2    // we don't have operator precedance
        *@.abs // it calculate one at first reg, and other at the second reg
        // and multiplies first and second reg, and expands f64.abs macro.

    perimeter: (addr @self =>f64)
        x1, x2 :: [self.p1]
        x2, y2 :: [self.p2]
        x1 - x2, y1 - y2
        .@abs, .@abs
        +
        * 2.0

    translate: (addr @self, f64 x, f64 y)
        [self.p1.x] + x =[]
        [self.p2.x] + x =[]
        [self.p1.y] + y =[]
        [self.p2.y] + y =[]

Pair:
    @struct {
        addr~ i32, addr~ i32 // ~ means it has to be freed by this pointer. kindof a owning pointer
    }

    destroy (~>@self)
        first, second :: self
        "Destroying Pair("first", "second")" print=>
        { first, second } do std.heap.free=>

i32 std.heap.alloc=> %
i32 std.heap.alloc=> %2
1 =[%]
2 =[%2]
pair :: Pair { %~, %2~ }    // ~ makes it own the pointer.
~>pair .destroy=>       // it uses move operator, the pointer inside pair does not have ownership, which accessing to it makes compile error.




