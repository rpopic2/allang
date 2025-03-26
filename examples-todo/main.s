    .globl _main
    .p2align 2
_main:
    sub sp, sp, 0x40
    stp x29, x30, [sp, 0x30]
    add x29, sp, 0x30

    cmp w0, 1
    bne check_cmd
    // list all
list_all:
    bl foo
read_file:
    adrp x0, filename@PAGE
    add x0, x0, filename@PAGEOFF
    adrp x1, mode_r@PAGE
    add x1, x1, mode_r@PAGEOFF
    bl _fopen
    str x0, [sp, 0x0]// 0x0: file

    mov x0, 1024
    bl _malloc
    str x0, [sp, 0x8]  // 0x8: buf

    mov x1, 0x1
    mov x2, 0x100
    ldr x3, [sp]
    bl _fread

    ldr x8, [sp, 0x8]
    str x8, [sp]
    adrp x0, format@PAGE
    add x0, x0, format@PAGEOFF
    // ldr x1, [sp, 0x8]
    bl _printf

check_cmd:
    mov w0, 0
epilogue:
    ldp x29, x30, [sp, 0x30]
    add sp, sp, 0x40
    ret

	.section	__TEXT,__cstring,cstring_literals
filename:
    .asciz "todo.txt"
mode_r:
    .asciz "r"
mode_w:
    .asciz "w"
mode_a:
    .asciz "a"
format:
    .asciz "conent: %s\n";
