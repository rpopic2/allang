	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	sub	sp, sp, #48
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	stur	wzr, [x29, #-4]
	mov	x9, sp
	sub	x8, x29, #8
	str	x8, [x9]
	sub	x8, x29, #12
	str	x8, [x9, #8]
	adrp	x0, l_.str@PAGE
	add	x0, x0, l_.str@PAGEOFF
	bl	_scanf
	ldur	w8, [x29, #-8]
	ldur	w9, [x29, #-12]
	subs	w8, w8, w9
	cset	w8, ge
	tbnz	w8, #0, LBB0_2
	b	LBB0_1
LBB0_1:
	mov	w8, #60                         ; =0x3c
	sturb	w8, [x29, #-13]
	b	LBB0_6
LBB0_2:
	ldur	w8, [x29, #-8]
	ldur	w9, [x29, #-12]
	subs	w8, w8, w9
	cset	w8, le
	tbnz	w8, #0, LBB0_4
	b	LBB0_3
LBB0_3:
	mov	w8, #62                         ; =0x3e
	sturb	w8, [x29, #-13]
	b	LBB0_5
LBB0_4:
	mov	w8, #61                         ; =0x3d
	sturb	w8, [x29, #-13]
	b	LBB0_5
LBB0_5:
	b	LBB0_6
LBB0_6:
	ldursb	w10, [x29, #-13]
	mov	x9, sp
                                        ; implicit-def: $x8
	mov	x8, x10
	str	x8, [x9]
	adrp	x0, l_.str.1@PAGE
	add	x0, x0, l_.str.1@PAGEOFF
	bl	_printf
	ldur	w0, [x29, #-4]
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret

                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"%d %d"

l_.str.1:                               ; @.str.1
	.asciz	"%c"

.subsections_via_symbols
