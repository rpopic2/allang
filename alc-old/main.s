	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 13, 0	sdk_version 14, 2
	.globl	_buffer_init                    ; -- Begin function buffer_init
	.p2align	2
_buffer_init:                           ; @buffer_init
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #32
	.cfi_def_cfa_offset 32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	str	x0, [sp, #8]
	mov	x0, #1024
	bl	_malloc
	str	x0, [sp]
	ldr	x8, [sp]
	ldr	x9, [sp, #8]
	str	x8, [x9]
	ldr	x8, [sp]
	ldr	x9, [sp, #8]
	str	x8, [x9, #8]
	ldr	x8, [sp]
	add	x8, x8, #1024
	ldr	x9, [sp, #8]
	str	x8, [x9, #16]
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #32
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_buffer_free                    ; -- Begin function buffer_free
	.p2align	2
_buffer_free:                           ; @buffer_free
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #32
	.cfi_def_cfa_offset 32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	str	x0, [sp, #8]
	ldr	x8, [sp, #8]
	ldr	x0, [x8]
	bl	_free
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #32
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_buffer_len                     ; -- Begin function buffer_len
	.p2align	2
_buffer_len:                            ; @buffer_len
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	x0, [sp, #8]
	ldr	x8, [sp, #8]
	ldr	x8, [x8, #8]
	ldr	x9, [sp, #8]
	ldr	x9, [x9]
	subs	x0, x8, x9
	add	sp, sp, #16
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_buffer_get_head                ; -- Begin function buffer_get_head
	.p2align	2
_buffer_get_head:                       ; @buffer_get_head
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	x0, [sp, #8]
	ldr	x8, [sp, #8]
	ldr	x0, [x8, #8]
	add	sp, sp, #16
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_buffer_puts                    ; -- Begin function buffer_puts
	.p2align	2
_buffer_puts:                           ; @buffer_puts
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	.cfi_def_cfa_offset 48
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	stur	x0, [x29, #-8]
	str	x1, [sp, #16]
	ldr	x0, [sp, #16]
	bl	_strlen
	str	x0, [sp, #8]
	ldur	x8, [x29, #-8]
	ldr	x0, [x8, #8]
	ldr	x1, [sp, #16]
	ldr	x2, [sp, #8]
	mov	x3, #-1
	bl	___memcpy_chk
	ldr	x10, [sp, #8]
	ldur	x9, [x29, #-8]
	ldr	x8, [x9, #8]
	add	x8, x8, x10
	str	x8, [x9, #8]
	ldur	x8, [x29, #-8]
	ldr	x8, [x8, #8]
	ldur	x9, [x29, #-8]
	ldr	x9, [x9, #16]
	subs	x8, x8, x9
	cset	w8, ls
	tbnz	w8, #0, LBB4_2
	b	LBB4_1
LBB4_1:
	adrp	x0, l_.str@PAGE
	add	x0, x0, l_.str@PAGEOFF
	bl	_printf
	bl	_abort
LBB4_2:
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_buffer_write_to_file           ; -- Begin function buffer_write_to_file
	.p2align	2
_buffer_write_to_file:                  ; @buffer_write_to_file
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #64
	.cfi_def_cfa_offset 64
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	stur	x0, [x29, #-16]
	str	x1, [sp, #24]
	ldr	x0, [sp, #24]
	adrp	x1, l_.str.1@PAGE
	add	x1, x1, l_.str.1@PAGEOFF
	bl	_fopen
	str	x0, [sp, #16]
	ldr	x8, [sp, #16]
	subs	x8, x8, #0
	cset	w8, ne
	tbnz	w8, #0, LBB5_2
	b	LBB5_1
LBB5_1:
	adrp	x0, l_.str.2@PAGE
	add	x0, x0, l_.str.2@PAGEOFF
	bl	_printf
	mov	w8, #-1
	stur	w8, [x29, #-4]
	b	LBB5_3
LBB5_2:
	ldur	x8, [x29, #-16]
	ldr	x8, [x8]
	str	x8, [sp, #8]                    ; 8-byte Folded Spill
	ldur	x0, [x29, #-16]
	bl	_buffer_len
	mov	x2, x0
	ldr	x0, [sp, #8]                    ; 8-byte Folded Reload
	ldr	x3, [sp, #16]
	mov	x1, #1
	bl	_fwrite
	ldr	x0, [sp, #16]
	bl	_fclose
	stur	wzr, [x29, #-4]
	b	LBB5_3
LBB5_3:
	ldur	w0, [x29, #-4]
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	add	sp, sp, #64
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_buffer_putc                    ; -- Begin function buffer_putc
	.p2align	2
_buffer_putc:                           ; @buffer_putc
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #32
	.cfi_def_cfa_offset 32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	str	x0, [sp, #8]
	strb	w1, [sp, #7]
	ldrb	w8, [sp, #7]
	ldr	x11, [sp, #8]
	ldr	x9, [x11, #8]
	add	x10, x9, #1
	str	x10, [x11, #8]
	strb	w8, [x9]
	ldr	x8, [sp, #8]
	ldr	x8, [x8, #8]
	ldr	x9, [sp, #8]
	ldr	x9, [x9, #16]
	subs	x8, x8, x9
	cset	w8, ls
	tbnz	w8, #0, LBB6_2
	b	LBB6_1
LBB6_1:
	adrp	x0, l_.str@PAGE
	add	x0, x0, l_.str@PAGEOFF
	bl	_printf
	bl	_abort
LBB6_2:
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #32
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #288
	.cfi_def_cfa_offset 288
	stp	x28, x27, [sp, #256]            ; 16-byte Folded Spill
	stp	x29, x30, [sp, #272]            ; 16-byte Folded Spill
	add	x29, sp, #272
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w27, -24
	.cfi_offset w28, -32
	adrp	x8, ___stack_chk_guard@GOTPAGE
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
	ldr	x8, [x8]
	stur	x8, [x29, #-24]
	stur	wzr, [x29, #-36]
	stur	w0, [x29, #-40]
	stur	x1, [x29, #-48]
	ldur	w8, [x29, #-40]
	subs	w8, w8, #2
	cset	w8, eq
	tbnz	w8, #0, LBB7_2
	b	LBB7_1
LBB7_1:
	adrp	x0, l_.str.10@PAGE
	add	x0, x0, l_.str.10@PAGEOFF
	bl	_printf
	mov	w8, #1
	stur	w8, [x29, #-36]
	b	LBB7_68
LBB7_2:
	ldur	x8, [x29, #-48]
	ldr	x8, [x8, #8]
	stur	x8, [x29, #-56]
	ldur	x0, [x29, #-56]
	bl	_strlen
	stur	x0, [x29, #-64]
	ldur	x0, [x29, #-56]
	adrp	x1, l_.str.11@PAGE
	add	x1, x1, l_.str.11@PAGEOFF
	bl	_fopen
	stur	x0, [x29, #-72]
	ldur	x8, [x29, #-72]
	subs	x8, x8, #0
	cset	w8, ne
	tbnz	w8, #0, LBB7_4
	b	LBB7_3
LBB7_3:
	adrp	x0, l_.str.12@PAGE
	add	x0, x0, l_.str.12@PAGEOFF
	bl	_printf
	mov	w8, #2
	stur	w8, [x29, #-36]
	b	LBB7_68
LBB7_4:
	sub	x0, x29, #96
	str	x0, [sp, #80]                   ; 8-byte Folded Spill
	bl	_buffer_init
	ldr	x0, [sp, #80]                   ; 8-byte Folded Reload
	adrp	x8, _header@PAGE
	ldr	x1, [x8, _header@PAGEOFF]
	bl	_buffer_puts
	stur	wzr, [x29, #-100]
	mov	x0, #64
	bl	_malloc
	stur	x0, [x29, #-112]
	stur	xzr, [x29, #-120]
	mov	w8, #48
	sturb	w8, [x29, #-121]
	stur	wzr, [x29, #-128]
	mov	w8, #16
	str	w8, [sp, #136]
	ldur	w8, [x29, #-128]
	str	w8, [sp, #132]
	str	wzr, [sp, #128]
	adrp	x0, l_.str.13@PAGE
	add	x0, x0, l_.str.13@PAGEOFF
	bl	_printf
	add	x0, sp, #120
	bl	_hashmap_new
	b	LBB7_5
LBB7_5:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB7_18 Depth 2
	ldur	x0, [x29, #-72]
	bl	_fgetc
	str	w0, [sp, #116]
	ldr	w8, [sp, #116]
	subs	w8, w8, #10
	cset	w8, eq
	and	w8, w8, #0x1
	str	w8, [sp, #112]
	ldr	w8, [sp, #116]
	subs	w8, w8, #32
	cset	w8, eq
	tbnz	w8, #0, LBB7_10
	b	LBB7_6
LBB7_6:                                 ;   in Loop: Header=BB7_5 Depth=1
	ldr	w8, [sp, #116]
	subs	w8, w8, #44
	cset	w8, eq
	tbnz	w8, #0, LBB7_10
	b	LBB7_7
LBB7_7:                                 ;   in Loop: Header=BB7_5 Depth=1
	ldr	w8, [sp, #112]
	subs	w8, w8, #0
	cset	w8, ne
	tbnz	w8, #0, LBB7_10
	b	LBB7_8
LBB7_8:                                 ;   in Loop: Header=BB7_5 Depth=1
	ldr	w8, [sp, #116]
	adds	w8, w8, #1
	cset	w8, eq
	tbnz	w8, #0, LBB7_10
	b	LBB7_9
LBB7_9:                                 ;   in Loop: Header=BB7_5 Depth=1
	ldr	w8, [sp, #116]
	ldur	x9, [x29, #-112]
	ldur	x10, [x29, #-120]
	add	x11, x10, #1
	stur	x11, [x29, #-120]
	strb	w8, [x9, x10]
	b	LBB7_5
LBB7_10:                                ;   in Loop: Header=BB7_5 Depth=1
	ldur	x8, [x29, #-120]
	subs	x8, x8, #0
	cset	w8, ne
	tbnz	w8, #0, LBB7_14
	b	LBB7_11
LBB7_11:                                ;   in Loop: Header=BB7_5 Depth=1
	ldr	w8, [sp, #116]
	adds	w8, w8, #1
	cset	w8, ne
	tbnz	w8, #0, LBB7_13
	b	LBB7_12
LBB7_12:
	b	LBB7_25
LBB7_13:                                ;   in Loop: Header=BB7_5 Depth=1
	b	LBB7_5
LBB7_14:                                ;   in Loop: Header=BB7_5 Depth=1
	ldur	x8, [x29, #-112]
	ldur	x9, [x29, #-120]
	add	x10, x9, #1
	stur	x10, [x29, #-120]
	add	x8, x8, x9
	strb	wzr, [x8]
	ldur	x8, [x29, #-112]
	mov	x9, sp
	str	x8, [x9]
	adrp	x0, l_.str.14@PAGE
	add	x0, x0, l_.str.14@PAGEOFF
	bl	_printf
	ldr	w8, [sp, #128]
	subs	w8, w8, #0
	cset	w8, eq
	tbnz	w8, #0, LBB7_16
	b	LBB7_15
LBB7_15:                                ;   in Loop: Header=BB7_5 Depth=1
	str	wzr, [sp, #128]
	b	LBB7_22
LBB7_16:                                ;   in Loop: Header=BB7_5 Depth=1
	ldur	x0, [x29, #-112]
	adrp	x1, l_.str.15@PAGE
	add	x1, x1, l_.str.15@PAGEOFF
	bl	_strcmp
	subs	w8, w0, #0
	cset	w8, ne
	tbnz	w8, #0, LBB7_21
	b	LBB7_17
LBB7_17:                                ;   in Loop: Header=BB7_5 Depth=1
	ldr	w8, [sp, #132]
	add	w8, w8, #4
	str	w8, [sp, #132]
	b	LBB7_18
LBB7_18:                                ;   Parent Loop BB7_5 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldr	w8, [sp, #132]
	ldur	w9, [x29, #-128]
	subs	w8, w8, w9
	cset	w8, le
	tbnz	w8, #0, LBB7_20
	b	LBB7_19
LBB7_19:                                ;   in Loop: Header=BB7_18 Depth=2
	ldur	w8, [x29, #-128]
	add	w8, w8, #16
	stur	w8, [x29, #-128]
	b	LBB7_18
LBB7_20:                                ;   in Loop: Header=BB7_5 Depth=1
	mov	w8, #1
	str	w8, [sp, #128]
	b	LBB7_21
LBB7_21:                                ;   in Loop: Header=BB7_5 Depth=1
	b	LBB7_22
LBB7_22:                                ;   in Loop: Header=BB7_5 Depth=1
	ldr	w8, [sp, #116]
	adds	w8, w8, #1
	cset	w8, ne
	tbnz	w8, #0, LBB7_24
	b	LBB7_23
LBB7_23:
	b	LBB7_25
LBB7_24:                                ;   in Loop: Header=BB7_5 Depth=1
	stur	xzr, [x29, #-120]
	b	LBB7_5
LBB7_25:
	ldur	w9, [x29, #-128]
                                        ; implicit-def: $x8
	mov	x8, x9
	mov	x9, sp
	str	x8, [x9]
	sub	x0, x29, #132
	mov	w1, #0
	mov	x2, #4
	adrp	x3, l_.str.16@PAGE
	add	x3, x3, l_.str.16@PAGEOFF
	bl	___sprintf_chk
	ldur	w9, [x29, #-128]
                                        ; implicit-def: $x8
	mov	x8, x9
	mov	x9, sp
	str	x8, [x9]
	adrp	x0, l_.str.17@PAGE
	add	x0, x0, l_.str.17@PAGEOFF
	bl	_printf
	ldur	w8, [x29, #-128]
	str	w8, [sp, #132]
	stur	xzr, [x29, #-120]
	ldur	x0, [x29, #-72]
	bl	_rewind
	adrp	x0, l_.str.18@PAGE
	add	x0, x0, l_.str.18@PAGEOFF
	bl	_printf
	b	LBB7_26
LBB7_26:                                ; =>This Inner Loop Header: Depth=1
	ldur	x0, [x29, #-72]
	bl	_fgetc
	str	w0, [sp, #108]
	ldr	w8, [sp, #108]
	subs	w8, w8, #10
	cset	w8, eq
	and	w8, w8, #0x1
	str	w8, [sp, #104]
	ldr	w8, [sp, #108]
	subs	w8, w8, #32
	cset	w8, eq
	tbnz	w8, #0, LBB7_31
	b	LBB7_27
LBB7_27:                                ;   in Loop: Header=BB7_26 Depth=1
	ldr	w8, [sp, #108]
	subs	w8, w8, #44
	cset	w8, eq
	tbnz	w8, #0, LBB7_31
	b	LBB7_28
LBB7_28:                                ;   in Loop: Header=BB7_26 Depth=1
	ldr	w8, [sp, #104]
	subs	w8, w8, #0
	cset	w8, ne
	tbnz	w8, #0, LBB7_31
	b	LBB7_29
LBB7_29:                                ;   in Loop: Header=BB7_26 Depth=1
	ldr	w8, [sp, #108]
	adds	w8, w8, #1
	cset	w8, eq
	tbnz	w8, #0, LBB7_31
	b	LBB7_30
LBB7_30:                                ;   in Loop: Header=BB7_26 Depth=1
	ldr	w8, [sp, #108]
	ldur	x9, [x29, #-112]
	ldur	x10, [x29, #-120]
	add	x11, x10, #1
	stur	x11, [x29, #-120]
	strb	w8, [x9, x10]
	b	LBB7_26
LBB7_31:                                ;   in Loop: Header=BB7_26 Depth=1
	ldur	x8, [x29, #-120]
	subs	x8, x8, #0
	cset	w8, ne
	tbnz	w8, #0, LBB7_35
	b	LBB7_32
LBB7_32:                                ;   in Loop: Header=BB7_26 Depth=1
	ldr	w8, [sp, #108]
	adds	w8, w8, #1
	cset	w8, ne
	tbnz	w8, #0, LBB7_34
	b	LBB7_33
LBB7_33:
	b	LBB7_67
LBB7_34:                                ;   in Loop: Header=BB7_26 Depth=1
	b	LBB7_26
LBB7_35:                                ;   in Loop: Header=BB7_26 Depth=1
	ldur	x8, [x29, #-112]
	ldur	x9, [x29, #-120]
	add	x10, x9, #1
	stur	x10, [x29, #-120]
	add	x8, x8, x9
	strb	wzr, [x8]
	ldur	x8, [x29, #-112]
	mov	x9, sp
	str	x8, [x9]
	adrp	x0, l_.str.14@PAGE
	add	x0, x0, l_.str.14@PAGEOFF
	bl	_printf
	ldur	w8, [x29, #-100]
	str	w8, [sp, #76]                   ; 4-byte Folded Spill
	subs	w8, w8, #0
	cset	w8, eq
	tbnz	w8, #0, LBB7_38
	b	LBB7_36
LBB7_36:                                ;   in Loop: Header=BB7_26 Depth=1
	ldr	w8, [sp, #76]                   ; 4-byte Folded Reload
	subs	w8, w8, #1
	cset	w8, eq
	tbnz	w8, #0, LBB7_57
	b	LBB7_37
LBB7_37:                                ;   in Loop: Header=BB7_26 Depth=1
	ldr	w8, [sp, #76]                   ; 4-byte Folded Reload
	subs	w8, w8, #2
	cset	w8, eq
	tbnz	w8, #0, LBB7_58
	b	LBB7_62
LBB7_38:                                ;   in Loop: Header=BB7_26 Depth=1
	ldur	x8, [x29, #-112]
	ldrsb	w8, [x8]
	subs	w8, w8, #48
	cset	w8, lt
	tbnz	w8, #0, LBB7_41
	b	LBB7_39
LBB7_39:                                ;   in Loop: Header=BB7_26 Depth=1
	ldur	x8, [x29, #-112]
	ldrsb	w8, [x8]
	subs	w8, w8, #57
	cset	w8, gt
	tbnz	w8, #0, LBB7_41
	b	LBB7_40
LBB7_40:                                ;   in Loop: Header=BB7_26 Depth=1
	adrp	x8, _ins_mov@PAGE
	ldr	x1, [x8, _ins_mov@PAGEOFF]
	sub	x0, x29, #96
	str	x0, [sp, #64]                   ; 8-byte Folded Spill
	bl	_buffer_puts
	ldr	x0, [sp, #64]                   ; 8-byte Folded Reload
	ldursb	w1, [x29, #-121]
	add	w8, w1, #1
	sturb	w8, [x29, #-121]
	bl	_buffer_putc
	ldr	x0, [sp, #64]                   ; 8-byte Folded Reload
	adrp	x1, l_.str.19@PAGE
	add	x1, x1, l_.str.19@PAGEOFF
	bl	_buffer_puts
	ldr	x0, [sp, #64]                   ; 8-byte Folded Reload
	ldur	x1, [x29, #-112]
	bl	_buffer_puts
	ldr	x0, [sp, #64]                   ; 8-byte Folded Reload
	mov	w1, #10
	bl	_buffer_putc
	b	LBB7_56
LBB7_41:                                ;   in Loop: Header=BB7_26 Depth=1
	ldur	x8, [x29, #-112]
	ldrsb	w8, [x8]
	subs	w8, w8, #43
	cset	w8, ne
	tbnz	w8, #0, LBB7_43
	b	LBB7_42
LBB7_42:                                ;   in Loop: Header=BB7_26 Depth=1
	adrp	x8, _ins_add@PAGE
	ldr	x1, [x8, _ins_add@PAGEOFF]
	sub	x0, x29, #96
	str	x0, [sp, #56]                   ; 8-byte Folded Spill
	bl	_buffer_puts
	ldr	x0, [sp, #56]                   ; 8-byte Folded Reload
	ldursb	w1, [x29, #-121]
	bl	_buffer_putc
	ldr	x0, [sp, #56]                   ; 8-byte Folded Reload
	adrp	x1, l_.str.20@PAGE
	add	x1, x1, l_.str.20@PAGEOFF
	bl	_buffer_puts
	b	LBB7_55
LBB7_43:                                ;   in Loop: Header=BB7_26 Depth=1
	ldur	x0, [x29, #-112]
	adrp	x1, l_.str.21@PAGE
	add	x1, x1, l_.str.21@PAGEOFF
	bl	_strcmp
	subs	w8, w0, #0
	cset	w8, ne
	tbnz	w8, #0, LBB7_45
	b	LBB7_44
LBB7_44:                                ;   in Loop: Header=BB7_26 Depth=1
	mov	w8, #48
	sturb	w8, [x29, #-121]
	b	LBB7_54
LBB7_45:                                ;   in Loop: Header=BB7_26 Depth=1
	ldur	x0, [x29, #-112]
	adrp	x1, l_.str.15@PAGE
	add	x1, x1, l_.str.15@PAGEOFF
	bl	_strcmp
	subs	w8, w0, #0
	cset	w8, ne
	tbnz	w8, #0, LBB7_47
	b	LBB7_46
LBB7_46:                                ;   in Loop: Header=BB7_26 Depth=1
	mov	w8, #2
	stur	w8, [x29, #-100]
	b	LBB7_53
LBB7_47:                                ;   in Loop: Header=BB7_26 Depth=1
	ldur	x0, [x29, #-112]
	adrp	x1, l_.str.22@PAGE
	add	x1, x1, l_.str.22@PAGEOFF
	bl	_strcmp
	subs	w8, w0, #0
	cset	w8, ne
	tbnz	w8, #0, LBB7_49
	b	LBB7_48
LBB7_48:                                ;   in Loop: Header=BB7_26 Depth=1
	mov	w8, #1
	stur	w8, [x29, #-100]
	b	LBB7_52
LBB7_49:                                ;   in Loop: Header=BB7_26 Depth=1
	ldur	x0, [x29, #-112]
	adrp	x1, l_.str.6@PAGE
	add	x1, x1, l_.str.6@PAGEOFF
	bl	_strcmp
	subs	w8, w0, #0
	cset	w8, ne
	tbnz	w8, #0, LBB7_51
	b	LBB7_50
LBB7_50:                                ;   in Loop: Header=BB7_26 Depth=1
	adrp	x8, _ins_add_sp@PAGE
	ldr	x1, [x8, _ins_add_sp@PAGEOFF]
	sub	x0, x29, #96
	str	x0, [sp, #48]                   ; 8-byte Folded Spill
	bl	_buffer_puts
	ldr	x0, [sp, #48]                   ; 8-byte Folded Reload
	sub	x1, x29, #132
	bl	_buffer_puts
	ldr	x0, [sp, #48]                   ; 8-byte Folded Reload
	adrp	x1, l_.str.23@PAGE
	add	x1, x1, l_.str.23@PAGEOFF
	bl	_buffer_puts
	ldr	x0, [sp, #48]                   ; 8-byte Folded Reload
	adrp	x8, _ins_ret@PAGE
	ldr	x1, [x8, _ins_ret@PAGEOFF]
	bl	_buffer_puts
	ldr	x0, [sp, #48]                   ; 8-byte Folded Reload
	mov	w1, #10
	bl	_buffer_putc
	b	LBB7_51
LBB7_51:                                ;   in Loop: Header=BB7_26 Depth=1
	b	LBB7_52
LBB7_52:                                ;   in Loop: Header=BB7_26 Depth=1
	b	LBB7_53
LBB7_53:                                ;   in Loop: Header=BB7_26 Depth=1
	b	LBB7_54
LBB7_54:                                ;   in Loop: Header=BB7_26 Depth=1
	b	LBB7_55
LBB7_55:                                ;   in Loop: Header=BB7_26 Depth=1
	b	LBB7_56
LBB7_56:                                ;   in Loop: Header=BB7_26 Depth=1
	b	LBB7_62
LBB7_57:                                ;   in Loop: Header=BB7_26 Depth=1
	sub	x0, x29, #96
	str	x0, [sp, #40]                   ; 8-byte Folded Spill
	mov	w1, #95
	bl	_buffer_putc
	ldr	x0, [sp, #40]                   ; 8-byte Folded Reload
	ldur	x1, [x29, #-112]
	bl	_buffer_puts
	ldr	x0, [sp, #40]                   ; 8-byte Folded Reload
	mov	w1, #10
	bl	_buffer_putc
	ldr	x0, [sp, #40]                   ; 8-byte Folded Reload
	adrp	x8, _ins_sub_sp@PAGE
	ldr	x1, [x8, _ins_sub_sp@PAGEOFF]
	bl	_buffer_puts
	ldr	x0, [sp, #40]                   ; 8-byte Folded Reload
	sub	x1, x29, #132
	bl	_buffer_puts
	ldr	x0, [sp, #40]                   ; 8-byte Folded Reload
	adrp	x1, l_.str.23@PAGE
	add	x1, x1, l_.str.23@PAGEOFF
	bl	_buffer_puts
	stur	wzr, [x29, #-100]
	b	LBB7_62
LBB7_58:                                ;   in Loop: Header=BB7_26 Depth=1
	ldur	x1, [x29, #-112]
	add	x0, sp, #120
	bl	_hashmap_get
	str	x0, [sp, #96]
	mov	w8, #48
	sturb	w8, [x29, #-121]
	adrp	x8, _ins_str@PAGE
	ldr	x1, [x8, _ins_str@PAGEOFF]
	sub	x0, x29, #96
	str	x0, [sp, #32]                   ; 8-byte Folded Spill
	bl	_buffer_puts
	ldr	x0, [sp, #32]                   ; 8-byte Folded Reload
	ldursb	w1, [x29, #-121]
	bl	_buffer_putc
	ldr	x0, [sp, #32]                   ; 8-byte Folded Reload
	adrp	x1, l_.str.24@PAGE
	add	x1, x1, l_.str.24@PAGEOFF
	bl	_buffer_puts
	ldr	x8, [sp, #96]
	subs	x8, x8, #0
	cset	w8, eq
	tbnz	w8, #0, LBB7_60
	b	LBB7_59
LBB7_59:                                ;   in Loop: Header=BB7_26 Depth=1
	ldr	x8, [sp, #96]
	mov	x9, sp
	str	x8, [x9]
	sub	x0, x29, #32
	mov	w1, #0
	mov	x2, #8
	adrp	x3, l_.str.25@PAGE
	add	x3, x3, l_.str.25@PAGEOFF
	bl	___sprintf_chk
	b	LBB7_61
LBB7_60:                                ;   in Loop: Header=BB7_26 Depth=1
	ldr	w8, [sp, #132]
	subs	w8, w8, #4
	str	w8, [sp, #132]
	ldr	w9, [sp, #132]
                                        ; implicit-def: $x8
	mov	x8, x9
	mov	x9, sp
	str	x8, [x9]
	sub	x0, x29, #32
	mov	w1, #0
	mov	x2, #8
	adrp	x3, l_.str.16@PAGE
	add	x3, x3, l_.str.16@PAGEOFF
	bl	___sprintf_chk
	ldur	x1, [x29, #-112]
	ldrsw	x2, [sp, #132]
	add	x0, sp, #120
	bl	_hashmap_add
	b	LBB7_61
LBB7_61:                                ;   in Loop: Header=BB7_26 Depth=1
	sub	x0, x29, #96
	str	x0, [sp, #24]                   ; 8-byte Folded Spill
	sub	x1, x29, #32
	bl	_buffer_puts
	ldr	x0, [sp, #24]                   ; 8-byte Folded Reload
	adrp	x1, l_.str.26@PAGE
	add	x1, x1, l_.str.26@PAGEOFF
	bl	_buffer_puts
	stur	wzr, [x29, #-100]
	b	LBB7_62
LBB7_62:                                ;   in Loop: Header=BB7_26 Depth=1
	ldr	w8, [sp, #108]
	adds	w8, w8, #1
	cset	w8, ne
	tbnz	w8, #0, LBB7_64
	b	LBB7_63
LBB7_63:
	b	LBB7_67
LBB7_64:                                ;   in Loop: Header=BB7_26 Depth=1
	stur	xzr, [x29, #-120]
	ldr	w8, [sp, #104]
	subs	w8, w8, #0
	cset	w8, eq
	tbnz	w8, #0, LBB7_66
	b	LBB7_65
LBB7_65:                                ;   in Loop: Header=BB7_26 Depth=1
	mov	w8, #48
	sturb	w8, [x29, #-121]
	b	LBB7_66
LBB7_66:                                ;   in Loop: Header=BB7_26 Depth=1
	b	LBB7_26
LBB7_67:
	ldur	x0, [x29, #-64]
	bl	_malloc
	str	x0, [sp, #88]
	ldr	x0, [sp, #88]
	ldur	x1, [x29, #-56]
	mov	x2, #-1
	bl	___strcpy_chk
	ldr	x8, [sp, #88]
	ldur	x9, [x29, #-64]
	subs	x9, x9, #2
	add	x9, x8, x9
	mov	w8, #115
	strb	w8, [x9]
	ldr	x8, [sp, #88]
	ldur	x9, [x29, #-64]
	subs	x9, x9, #1
	add	x8, x8, x9
	strb	wzr, [x8]
	sub	x0, x29, #96
	str	x0, [sp, #16]                   ; 8-byte Folded Spill
	mov	w1, #10
	bl	_buffer_putc
	ldr	x0, [sp, #16]                   ; 8-byte Folded Reload
	adrp	x1, l_.str.27@PAGE
	add	x1, x1, l_.str.27@PAGEOFF
	bl	_buffer_write_to_file
	ldur	x0, [x29, #-72]
	bl	_fclose
	ldr	x0, [sp, #16]                   ; 8-byte Folded Reload
	bl	_buffer_free
	add	x0, sp, #120
	bl	_hashmap_free
	ldur	x0, [x29, #-112]
	bl	_free
	stur	wzr, [x29, #-36]
	b	LBB7_68
LBB7_68:
	ldur	w8, [x29, #-36]
	str	w8, [sp, #12]                   ; 4-byte Folded Spill
	ldur	x9, [x29, #-24]
	adrp	x8, ___stack_chk_guard@GOTPAGE
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
	ldr	x8, [x8]
	subs	x8, x8, x9
	cset	w8, eq
	tbnz	w8, #0, LBB7_70
	b	LBB7_69
LBB7_69:
	bl	___stack_chk_fail
LBB7_70:
	ldr	w0, [sp, #12]                   ; 4-byte Folded Reload
	ldp	x29, x30, [sp, #272]            ; 16-byte Folded Reload
	ldp	x28, x27, [sp, #256]            ; 16-byte Folded Reload
	add	sp, sp, #288
	ret
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"buffer overflow!"

l_.str.1:                               ; @.str.1
	.asciz	"w"

l_.str.2:                               ; @.str.2
	.asciz	"failed to create object file"

l_.str.3:                               ; @.str.3
	.asciz	".global _main\n.p2align 2\n\n"

	.section	__DATA,__data
	.globl	_header                         ; @header
	.p2align	3, 0x0
_header:
	.quad	l_.str.3

	.section	__TEXT,__cstring,cstring_literals
l_.str.4:                               ; @.str.4
	.asciz	"mov w"

	.section	__DATA,__data
	.globl	_ins_mov                        ; @ins_mov
	.p2align	3, 0x0
_ins_mov:
	.quad	l_.str.4

	.section	__TEXT,__cstring,cstring_literals
l_.str.5:                               ; @.str.5
	.asciz	"add w"

	.section	__DATA,__data
	.globl	_ins_add                        ; @ins_add
	.p2align	3, 0x0
_ins_add:
	.quad	l_.str.5

	.section	__TEXT,__cstring,cstring_literals
l_.str.6:                               ; @.str.6
	.asciz	"ret"

	.section	__DATA,__data
	.globl	_ins_ret                        ; @ins_ret
	.p2align	3, 0x0
_ins_ret:
	.quad	l_.str.6

	.section	__TEXT,__cstring,cstring_literals
l_.str.7:                               ; @.str.7
	.asciz	"str w"

	.section	__DATA,__data
	.globl	_ins_str                        ; @ins_str
	.p2align	3, 0x0
_ins_str:
	.quad	l_.str.7

	.section	__TEXT,__cstring,cstring_literals
l_.str.8:                               ; @.str.8
	.asciz	"add sp, sp, #0x"

	.section	__DATA,__data
	.globl	_ins_add_sp                     ; @ins_add_sp
	.p2align	3, 0x0
_ins_add_sp:
	.quad	l_.str.8

	.section	__TEXT,__cstring,cstring_literals
l_.str.9:                               ; @.str.9
	.asciz	"sub sp, sp, #0x"

	.section	__DATA,__data
	.globl	_ins_sub_sp                     ; @ins_sub_sp
	.p2align	3, 0x0
_ins_sub_sp:
	.quad	l_.str.9

	.section	__TEXT,__cstring,cstring_literals
l_.str.10:                              ; @.str.10
	.asciz	"please provide source file.\n"

l_.str.11:                              ; @.str.11
	.asciz	"r"

l_.str.12:                              ; @.str.12
	.asciz	"source file does not exist\n"

l_.str.13:                              ; @.str.13
	.asciz	"stack sentry phase\n"

l_.str.14:                              ; @.str.14
	.asciz	"[%s]\n"

l_.str.15:                              ; @.str.15
	.asciz	"=>"

l_.str.16:                              ; @.str.16
	.asciz	"%x"

l_.str.17:                              ; @.str.17
	.asciz	"stack size: %x\n"

l_.str.18:                              ; @.str.18
	.asciz	"compile phase\n"

l_.str.19:                              ; @.str.19
	.asciz	", #"

l_.str.20:                              ; @.str.20
	.asciz	", w0, w1\n"

l_.str.21:                              ; @.str.21
	.asciz	"->"

l_.str.22:                              ; @.str.22
	.asciz	"fn"

l_.str.23:                              ; @.str.23
	.asciz	"\n"

l_.str.24:                              ; @.str.24
	.asciz	", [sp, #0x"

l_.str.25:                              ; @.str.25
	.asciz	"%zx"

l_.str.26:                              ; @.str.26
	.asciz	"]\n"

l_.str.27:                              ; @.str.27
	.asciz	"a.s"

.subsections_via_symbols
