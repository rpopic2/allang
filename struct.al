struct Vec2 {
    i32 a, b
}

struct gen_ptr {
    type t, ptr p
}

ptr::Vec2 Vec2::add:
    ptr::Vec2 self, ptr::Vec2 rhs
    rhs => Vec2 => rhs
    rhs.a => [self.a]
    rhs.b => [self.b]
    self -> ret

fn main:
    3, 5 -> Vec2 => v1
    10 => v1.a
    10, 8 -> Vec2 => v2
    & v1, & v2 -> Vec2::add
    & v2 -> v1::add
    -> print

