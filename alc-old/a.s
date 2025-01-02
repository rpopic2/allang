.global _main
.p2align 2

_main:
sub sp, sp, #0x10
mov w0, #1
str w0, [sp, #0xc]
mov w0, #2
str w0, [sp, #0x8]
add w0, w0, w1
str w0, [sp, #0x4]
add sp, sp, #0x10
ret

