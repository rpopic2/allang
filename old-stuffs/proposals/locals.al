fn main:
    0, 3 (i32) => a, b
    + => sum
    a, b, sum -> f "sum of % and % is %\n" -> print
    0 -> ret

//EOF
in c:

int main(void) {
    int a = 0, b = 3;
    int sum = a + b;
    printf("sum of %d and %d is %d\n", a, b, sum);
    return 0;
}

will compile to:
_main:
mov w0, 0
mov w1, 3
str w0, [sp]
str w1, [sp, #4]
add w0, w0, w1
str w0, [sp, #8]
ldr w0, [sp]
ldr w1, [sp, #4]
ldr w2, [sp, #8]
b fmt
b print
ret
