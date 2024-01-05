fn main:
    1, 2, 3, 4
    ax, ay, bx, by
    add_vec2
    +
    ret

fn add_vec2 -> i32, i32:
    ax, ay, bx, by (i32)
    ax, bx -> + => sx
    ay, by -> + => sy
    sx, sy -> ret

(int, int) add_vec2(int ax, int ay, int bx, int by) {
    int sx = ax + bx;
    int sy = ay + by;
    return (sy, sy);
}

int main(void) {
    int ax = 1, ay = 2, bx = 3, by = 4;
    add_vec2(ax, ay, bx, by);
}
