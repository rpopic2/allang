proc scan.i32:
    __platform aarch64 "asos" << EOF

    ::asos::peripherals

    acc: (i32)
    0 ->acc
.loop:
    tmp: (u8)
    [uart] ->tmp
    tmp + acc ->acc
    ? tmp, ' '
        != :: .loop
    acc
    ret

    EOF

    __asm << EOF

    mov acc, #0x0
loop:
    ldrb tmp, [x18]
    add acc, acc, tmp
    cmp acc, 0x20
    b.ne loop

    EOF

proc main:
    &a, &b
    / scan.i32 ->a, / scan.i32 ->b
    cmp
        :: < '<'
        :: > '>'
        :: _ '='
    print

    // aarch64 2

main:
    bl scan
    bl toint
    mov w19, w0

    bl scan
    bl toint
    mov w20, w1

    cmp w19, w20

    mov w8, 0x3c    // <: 0x3c, =: 0x3d, >: 0x3e
    cinc w8, ge
    cinc w8, gt

    bl print
    ret

    // aarch64
main:
    bl scan
    bl toint
    mov w19, w0

    bl scan
    bl toint
    mov w20, w0

    cmp w19, w20
    blt LBB0
    bge LBB1
    b LBB3
LBB0:
    mov w0, 0x3c // '<'
    b LBB2
LBB1:
    mov w0, 0x3e // '>'
    b LBB2
LBB3:
    mov w0, 0x3d // '='
    b LBB2
LBB2:
    bl print
    ret

    // c

#include <stdio.h>

int main(void) {
    int a, b;
    scanf("%d %d", &a, &b);
    char c;
    if (a < b) {
        c = '<';
    } else if (a > b) {
        c = '>';
    } else {
        c = '=';
    }
    printf(c);
}

