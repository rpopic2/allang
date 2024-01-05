struct vec2 {
    i32 x, y
}

fn main:
    (1, 2 :vec2), (3, 4 :vec2)
    add_vec2 => sum
    .x, .y -> +
    ret

fn add_vec2 -> vec2:
    a, b :vec2
    (a.x, b.x -> +), (a.y, b.y -> +) -> vec2 sum
    ret

typedef struct {
    int x, y;
} vec2;

vec2 add_vec2(vec2 a, vec2 b) {
    vec2 sum;
    sum.x = a.x + b.x;
    sum.y = a.y + b.y;
    return sum;
}

int main(void) {
    vec2 a = { .x = 1, .y = 2 };
    vec2 b = { .x = 3, .y = 4 };
    vec2 sum = add_vec2(a, b);
    return sum.x + sum.y;
}
