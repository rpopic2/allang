	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	sub	sp, sp, #48
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	sub	x8, x29, #8
	sub	x9, x29, #4
	stp	x9, x8, [sp]
Lloh0:
	adrp	x0, l_.str@PAGE
Lloh1:
	add	x0, x0, l_.str@PAGEOFF
	bl	_scanf
	ldp	w9, w8, [x29, #-8]
	mov	w10, #61                        ; =0x3d
	mov	w11, #60                        ; =0x3c
	cmp	w8, w9
	cinc	w8, w10, gt
	csel	w0, w11, w8, lt
	bl	_putchar
	mov	w0, #0                          ; =0x0
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"%d %d"
