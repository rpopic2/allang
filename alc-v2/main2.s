	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 14, 0	sdk_version 15, 2
	.globl	_lex                            ; -- Begin function lex
	.p2align	2
_lex:                                   ; @lex
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #64
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	stur	x0, [x29, #-8]
	stur	x1, [x29, #-16]
	b	LBB0_1
LBB0_1:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_2 Depth 2
                                        ;       Child Loop BB0_12 Depth 3
                                        ;     Child Loop BB0_4 Depth 2
	ldur	x8, [x29, #-8]
	ldur	x9, [x29, #-16]
	ldr	x9, [x9]
	str	x9, [sp, #16]
                                        ; kill: def $x9 killed $xzr
	str	xzr, [sp, #24]
	ldr	q0, [sp, #16]
	str	q0, [x8]
	b	LBB0_2
LBB0_2:                                 ;   Parent Loop BB0_1 Depth=1
                                        ; =>  This Loop Header: Depth=2
                                        ;       Child Loop BB0_12 Depth 3
	ldur	x8, [x29, #-16]
	ldr	x8, [x8]
	ldrb	w8, [x8]
	strb	w8, [sp, #15]
	ldrsb	w8, [sp, #15]
	subs	w8, w8, #34
	cset	w8, ne
	tbnz	w8, #0, LBB0_9
	b	LBB0_3
LBB0_3:                                 ;   in Loop: Header=BB0_1 Depth=1
	b	LBB0_4
LBB0_4:                                 ;   Parent Loop BB0_1 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldur	x10, [x29, #-16]
	ldr	x8, [x10]
	add	x9, x8, #1
	str	x9, [x10]
	ldrb	w8, [x8, #1]
	strb	w8, [sp, #15]
	b	LBB0_5
LBB0_5:                                 ;   in Loop: Header=BB0_4 Depth=2
	ldrsb	w8, [sp, #15]
	subs	w8, w8, #34
	cset	w8, eq
	mov	w9, #0                          ; =0x0
	str	w9, [sp, #8]                    ; 4-byte Folded Spill
	tbnz	w8, #0, LBB0_7
	b	LBB0_6
LBB0_6:                                 ;   in Loop: Header=BB0_4 Depth=2
	ldrsb	w8, [sp, #15]
	subs	w8, w8, #10
	cset	w8, ne
	str	w8, [sp, #8]                    ; 4-byte Folded Spill
	b	LBB0_7
LBB0_7:                                 ;   in Loop: Header=BB0_4 Depth=2
	ldr	w8, [sp, #8]                    ; 4-byte Folded Reload
	tbnz	w8, #0, LBB0_4
	b	LBB0_8
LBB0_8:                                 ;   in Loop: Header=BB0_1 Depth=1
	ldur	x10, [x29, #-16]
	ldr	x8, [x10]
	add	x8, x8, #1
	mov	x9, x8
	str	x9, [x10]
	ldur	x9, [x29, #-8]
	str	x8, [x9, #8]
	b	LBB0_25
LBB0_9:                                 ;   in Loop: Header=BB0_2 Depth=2
	ldrsb	w8, [sp, #15]
	subs	w8, w8, #47
	cset	w8, ne
	tbnz	w8, #0, LBB0_15
	b	LBB0_10
LBB0_10:                                ;   in Loop: Header=BB0_2 Depth=2
	ldur	x8, [x29, #-16]
	ldr	x8, [x8]
	ldrsb	w8, [x8, #1]
	subs	w8, w8, #47
	cset	w8, ne
	tbnz	w8, #0, LBB0_15
	b	LBB0_11
LBB0_11:                                ;   in Loop: Header=BB0_2 Depth=2
	b	LBB0_12
LBB0_12:                                ;   Parent Loop BB0_1 Depth=1
                                        ;     Parent Loop BB0_2 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	ldur	x10, [x29, #-16]
	ldr	x8, [x10]
	add	x9, x8, #1
	str	x9, [x10]
	ldrb	w8, [x8, #1]
	strb	w8, [sp, #15]
	b	LBB0_13
LBB0_13:                                ;   in Loop: Header=BB0_12 Depth=3
	ldrsb	w8, [sp, #15]
	subs	w8, w8, #10
	cset	w8, ne
	tbnz	w8, #0, LBB0_12
	b	LBB0_14
LBB0_14:                                ;   in Loop: Header=BB0_2 Depth=2
	ldur	x8, [x29, #-16]
	ldr	x8, [x8]
	ldur	x9, [x29, #-8]
	str	x8, [x9]
	b	LBB0_15
LBB0_15:                                ;   in Loop: Header=BB0_2 Depth=2
	ldur	x8, [x29, #-16]
	ldr	x8, [x8]
	ldrsb	w8, [x8]
	subs	w8, w8, #10
	cset	w8, ne
	tbnz	w8, #0, LBB0_17
	b	LBB0_16
LBB0_16:                                ;   in Loop: Header=BB0_2 Depth=2
	adrp	x9, _lineno@PAGE
	ldr	w8, [x9, _lineno@PAGEOFF]
	add	w8, w8, #1
	str	w8, [x9, _lineno@PAGEOFF]
	b	LBB0_17
LBB0_17:                                ;   in Loop: Header=BB0_2 Depth=2
	ldrsb	w8, [sp, #15]
	subs	w8, w8, #44
	cset	w8, eq
	tbnz	w8, #0, LBB0_21
	b	LBB0_18
LBB0_18:                                ;   in Loop: Header=BB0_2 Depth=2
	ldrsb	w8, [sp, #15]
	subs	w8, w8, #10
	cset	w8, eq
	tbnz	w8, #0, LBB0_21
	b	LBB0_19
LBB0_19:                                ;   in Loop: Header=BB0_2 Depth=2
	ldrsb	w8, [sp, #15]
	subs	w8, w8, #32
	cset	w8, eq
	tbnz	w8, #0, LBB0_21
	b	LBB0_20
LBB0_20:                                ;   in Loop: Header=BB0_2 Depth=2
	ldrsb	w8, [sp, #15]
	subs	w8, w8, #0
	cset	w8, ne
	tbnz	w8, #0, LBB0_24
	b	LBB0_21
LBB0_21:                                ;   in Loop: Header=BB0_1 Depth=1
	ldur	x10, [x29, #-16]
	ldr	x8, [x10]
	mov	x9, x8
	add	x9, x9, #1
	str	x9, [x10]
	ldur	x9, [x29, #-8]
	str	x8, [x9, #8]
	ldrsb	w8, [sp, #15]
	subs	w8, w8, #44
	cset	w8, ne
	tbnz	w8, #0, LBB0_23
	b	LBB0_22
LBB0_22:                                ;   in Loop: Header=BB0_1 Depth=1
	ldur	x9, [x29, #-16]
	ldr	x8, [x9]
	add	x8, x8, #1
	str	x8, [x9]
	b	LBB0_23
LBB0_23:                                ;   in Loop: Header=BB0_1 Depth=1
	b	LBB0_25
LBB0_24:                                ;   in Loop: Header=BB0_2 Depth=2
	ldur	x9, [x29, #-16]
	ldr	x8, [x9]
	add	x8, x8, #1
	str	x8, [x9]
	b	LBB0_2
LBB0_25:                                ;   in Loop: Header=BB0_1 Depth=1
	ldur	x8, [x29, #-8]
	ldr	x8, [x8, #8]
	ldur	x9, [x29, #-16]
	ldr	x9, [x9, #16]
	subs	x8, x8, x9
	cset	w8, ls
	tbnz	w8, #0, LBB0_27
	b	LBB0_26
LBB0_26:
	ldur	x8, [x29, #-8]
	adrp	x9, _str_null@PAGE
	add	x9, x9, _str_null@PAGEOFF
	ldr	q0, [x9]
	str	q0, [x8]
	b	LBB0_29
LBB0_27:                                ;   in Loop: Header=BB0_1 Depth=1
	ldur	x0, [x29, #-8]
	bl	_str_len
	subs	x8, x0, #0
	cset	w8, ne
	tbnz	w8, #0, LBB0_29
	b	LBB0_28
LBB0_28:                                ;   in Loop: Header=BB0_1 Depth=1
	b	LBB0_1
LBB0_29:
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	add	sp, sp, #64
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_compile_err                    ; -- Begin function compile_err
	.p2align	2
_compile_err:                           ; @compile_err
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	stur	x0, [x29, #-8]
	adrp	x9, _has_compile_err@PAGE
	mov	w8, #1                          ; =0x1
	strb	w8, [x9, _has_compile_err@PAGEOFF]
	adrp	x8, ___stderrp@GOTPAGE
	ldr	x8, [x8, ___stderrp@GOTPAGEOFF]
	str	x8, [sp, #8]                    ; 8-byte Folded Spill
	ldr	x1, [x8]
	adrp	x0, l_.str@PAGE
	add	x0, x0, l_.str@PAGEOFF
	bl	_fputs
	ldr	x8, [sp, #8]                    ; 8-byte Folded Reload
	ldr	x0, [x8]
	adrp	x8, _lineno@PAGE
	ldr	w9, [x8, _lineno@PAGEOFF]
                                        ; implicit-def: $x8
	mov	x8, x9
	mov	x9, sp
	str	x8, [x9]
	adrp	x1, l_.str.1@PAGE
	add	x1, x1, l_.str.1@PAGEOFF
	bl	_fprintf
	ldr	x8, [sp, #8]                    ; 8-byte Folded Reload
	add	x10, sp, #16
	add	x9, x29, #16
	str	x9, [x10]
	ldr	x0, [x8]
	ldur	x1, [x29, #-8]
	ldr	x2, [sp, #16]
	bl	_vfprintf
	ldr	x8, [sp, #8]                    ; 8-byte Folded Reload
	ldr	x1, [x8]
	adrp	x0, l_.str.2@PAGE
	add	x0, x0, l_.str.2@PAGEOFF
	bl	_fputs
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_compile_warning                ; -- Begin function compile_warning
	.p2align	2
_compile_warning:                       ; @compile_warning
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	stur	x0, [x29, #-8]
	adrp	x8, ___stderrp@GOTPAGE
	ldr	x8, [x8, ___stderrp@GOTPAGEOFF]
	str	x8, [sp, #8]                    ; 8-byte Folded Spill
	ldr	x1, [x8]
	adrp	x0, l_.str.3@PAGE
	add	x0, x0, l_.str.3@PAGEOFF
	bl	_fputs
	ldr	x8, [sp, #8]                    ; 8-byte Folded Reload
	ldr	x0, [x8]
	adrp	x8, _lineno@PAGE
	ldr	w9, [x8, _lineno@PAGEOFF]
                                        ; implicit-def: $x8
	mov	x8, x9
	mov	x9, sp
	str	x8, [x9]
	adrp	x1, l_.str.4@PAGE
	add	x1, x1, l_.str.4@PAGEOFF
	bl	_fprintf
	ldr	x8, [sp, #8]                    ; 8-byte Folded Reload
	add	x10, sp, #16
	add	x9, x29, #16
	str	x9, [x10]
	ldr	x0, [x8]
	ldur	x1, [x29, #-8]
	ldr	x2, [sp, #16]
	bl	_vfprintf
	ldr	x8, [sp, #8]                    ; 8-byte Folded Reload
	ldr	x1, [x8]
	adrp	x0, l_.str.2@PAGE
	add	x0, x0, l_.str.2@PAGEOFF
	bl	_fputs
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_literal_string                 ; -- Begin function literal_string
	.p2align	2
_literal_string:                        ; @literal_string
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #128
	stp	x29, x30, [sp, #112]            ; 16-byte Folded Spill
	add	x29, sp, #112
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	stur	x0, [x29, #-8]
	stur	x1, [x29, #-16]
	bl	_emit_need_escaping
	mov	w8, #1                          ; =0x1
	and	w8, w0, w8
	sturb	w8, [x29, #-17]
	ldur	x8, [x29, #-16]
	ldr	x8, [x8, #8]
	ldursb	w8, [x8, #-1]
	subs	w8, w8, #34
	cset	w8, eq
	tbnz	w8, #0, LBB3_2
	b	LBB3_1
LBB3_1:
	adrp	x0, l_.str.5@PAGE
	add	x0, x0, l_.str.5@PAGEOFF
	bl	_compile_err
	b	LBB3_2
LBB3_2:
	ldurb	w8, [x29, #-17]
	tbnz	w8, #0, LBB3_4
	b	LBB3_3
LBB3_3:
	ldur	x8, [x29, #-8]
	ldr	w0, [x8]
	ldur	x8, [x29, #-8]
	ldr	w1, [x8, #4]
	ldur	x2, [x29, #-16]
	bl	_emit_string_lit
	b	LBB3_24
LBB3_4:
	ldur	x0, [x29, #-16]
	bl	_str_len
	stur	x0, [x29, #-32]
	ldur	x0, [x29, #-32]
	bl	_malloc
	ldur	x1, [x29, #-32]
	add	x8, sp, #56
	bl	_iter_init
	str	wzr, [sp, #52]
	b	LBB3_5
LBB3_5:                                 ; =>This Inner Loop Header: Depth=1
	ldrsw	x8, [sp, #52]
	ldur	x9, [x29, #-32]
	subs	x8, x8, x9
	cset	w8, hs
	tbnz	w8, #0, LBB3_23
	b	LBB3_6
LBB3_6:                                 ;   in Loop: Header=BB3_5 Depth=1
	ldur	x8, [x29, #-16]
	ldr	x8, [x8]
	ldrsw	x9, [sp, #52]
	add	x8, x8, x9
	ldrb	w8, [x8]
	strb	w8, [sp, #51]
	ldrsb	w8, [sp, #51]
	subs	w8, w8, #92
	cset	w8, eq
	tbnz	w8, #0, LBB3_8
	b	LBB3_7
LBB3_7:                                 ;   in Loop: Header=BB3_5 Depth=1
	ldrb	w8, [sp, #51]
	ldr	x9, [sp, #56]
	add	x10, x9, #1
	str	x10, [sp, #56]
	strb	w8, [x9]
	b	LBB3_21
LBB3_8:                                 ;   in Loop: Header=BB3_5 Depth=1
	ldur	x8, [x29, #-16]
	ldr	x8, [x8]
	ldr	w9, [sp, #52]
	add	w9, w9, #1
	str	w9, [sp, #52]
	add	x8, x8, w9, sxtw
	ldrb	w8, [x8]
	strb	w8, [sp, #51]
	mov	w8, #32                         ; =0x20
	strb	w8, [sp, #50]
	ldrsb	w8, [sp, #51]
	str	w8, [sp, #20]                   ; 4-byte Folded Spill
	subs	w8, w8, #48
	cset	w8, eq
	tbnz	w8, #0, LBB3_14
	b	LBB3_9
LBB3_9:                                 ;   in Loop: Header=BB3_5 Depth=1
	ldr	w8, [sp, #20]                   ; 4-byte Folded Reload
	subs	w8, w8, #92
	cset	w8, eq
	tbnz	w8, #0, LBB3_15
	b	LBB3_10
LBB3_10:                                ;   in Loop: Header=BB3_5 Depth=1
	ldr	w8, [sp, #20]                   ; 4-byte Folded Reload
	subs	w8, w8, #110
	cset	w8, eq
	tbnz	w8, #0, LBB3_12
	b	LBB3_11
LBB3_11:                                ;   in Loop: Header=BB3_5 Depth=1
	ldr	w8, [sp, #20]                   ; 4-byte Folded Reload
	subs	w8, w8, #116
	cset	w8, eq
	tbnz	w8, #0, LBB3_13
	b	LBB3_16
LBB3_12:                                ;   in Loop: Header=BB3_5 Depth=1
	mov	w8, #10                         ; =0xa
	strb	w8, [sp, #50]
	b	LBB3_20
LBB3_13:                                ;   in Loop: Header=BB3_5 Depth=1
	mov	w8, #9                          ; =0x9
	strb	w8, [sp, #50]
	b	LBB3_20
LBB3_14:                                ;   in Loop: Header=BB3_5 Depth=1
	strb	wzr, [sp, #50]
	b	LBB3_20
LBB3_15:                                ;   in Loop: Header=BB3_5 Depth=1
	mov	w8, #92                         ; =0x5c
	strb	w8, [sp, #50]
	b	LBB3_20
LBB3_16:                                ;   in Loop: Header=BB3_5 Depth=1
	ldur	x8, [x29, #-16]
	ldr	x0, [x8]
	mov	x1, #0                          ; =0x0
	mov	w2, #0                          ; =0x0
	bl	_strtol
	str	x0, [sp, #40]
	ldr	x8, [sp, #40]
	subs	x8, x8, #1
	cset	w8, ls
	tbnz	w8, #0, LBB3_18
	b	LBB3_17
LBB3_17:                                ;   in Loop: Header=BB3_5 Depth=1
	ldr	x8, [sp, #40]
	mov	x9, sp
	str	x8, [x9]
	adrp	x0, l_.str.6@PAGE
	add	x0, x0, l_.str.6@PAGEOFF
	bl	_compile_err
	b	LBB3_19
LBB3_18:                                ;   in Loop: Header=BB3_5 Depth=1
	ldr	x8, [sp, #40]
                                        ; kill: def $w8 killed $w8 killed $x8
	strb	w8, [sp, #50]
	b	LBB3_19
LBB3_19:                                ;   in Loop: Header=BB3_5 Depth=1
	b	LBB3_20
LBB3_20:                                ;   in Loop: Header=BB3_5 Depth=1
	ldrb	w8, [sp, #50]
	ldr	x9, [sp, #56]
	add	x10, x9, #1
	str	x10, [sp, #56]
	strb	w8, [x9]
	b	LBB3_21
LBB3_21:                                ;   in Loop: Header=BB3_5 Depth=1
	b	LBB3_22
LBB3_22:                                ;   in Loop: Header=BB3_5 Depth=1
	ldr	w8, [sp, #52]
	add	w8, w8, #1
	str	w8, [sp, #52]
	b	LBB3_5
LBB3_23:
	add	x0, sp, #56
	bl	_str_from_iter
	add	x2, sp, #24
	str	x0, [sp, #24]
	str	x1, [sp, #32]
	ldur	x8, [x29, #-8]
	ldr	w0, [x8]
	ldur	x8, [x29, #-8]
	ldr	w1, [x8, #4]
	bl	_emit_string_lit
	ldr	x0, [sp, #64]
	bl	_free
	b	LBB3_24
LBB3_24:
	ldp	x29, x30, [sp, #112]            ; 16-byte Folded Reload
	add	sp, sp, #128
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_lit_numeric                    ; -- Begin function lit_numeric
	.p2align	2
_lit_numeric:                           ; @lit_numeric
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #64
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	str	x0, [sp, #24]
	str	xzr, [sp, #16]
	ldr	x8, [sp, #24]
	ldr	x8, [x8]
	ldrsb	w0, [x8]
	bl	_isdigit
	subs	w8, w0, #0
	cset	w8, eq
	tbnz	w8, #0, LBB4_2
	b	LBB4_1
LBB4_1:
	ldr	x8, [sp, #24]
	ldr	x0, [x8]
	mov	x1, #0                          ; =0x0
	mov	w2, #0                          ; =0x0
	bl	_strtol
	str	x0, [sp, #16]
	b	LBB4_14
LBB4_2:
	ldr	x8, [sp, #24]
	ldr	x8, [x8]
	ldrsb	w8, [x8]
	subs	w8, w8, #39
	cset	w8, ne
	tbnz	w8, #0, LBB4_6
	b	LBB4_3
LBB4_3:
	ldr	x8, [sp, #24]
	ldr	x8, [x8]
	ldrb	w8, [x8, #1]
	strb	w8, [sp, #15]
	ldr	x8, [sp, #24]
	ldr	x8, [x8, #8]
	ldursb	w8, [x8, #-1]
	subs	w8, w8, #39
	cset	w8, eq
	tbnz	w8, #0, LBB4_5
	b	LBB4_4
LBB4_4:
	adrp	x0, l_.str.7@PAGE
	add	x0, x0, l_.str.7@PAGEOFF
	bl	_compile_err
	b	LBB4_5
LBB4_5:
	ldrsb	x8, [sp, #15]
	str	x8, [sp, #16]
	b	LBB4_13
LBB4_6:
	ldr	x0, [sp, #24]
	adrp	x1, l_.str.8@PAGE
	add	x1, x1, l_.str.8@PAGEOFF
	bl	_str_eq_lit
	tbz	w0, #0, LBB4_8
	b	LBB4_7
LBB4_7:
	mov	x8, #1                          ; =0x1
	str	x8, [sp, #16]
	b	LBB4_12
LBB4_8:
	ldr	x0, [sp, #24]
	adrp	x1, l_.str.9@PAGE
	add	x1, x1, l_.str.9@PAGEOFF
	bl	_str_eq_lit
	tbz	w0, #0, LBB4_10
	b	LBB4_9
LBB4_9:
	str	xzr, [sp, #16]
	b	LBB4_11
LBB4_10:
	adrp	x8, _opt_long_none@PAGE
	add	x8, x8, _opt_long_none@PAGEOFF
	ldr	q0, [x8]
	stur	q0, [x29, #-16]
	b	LBB4_15
LBB4_11:
	b	LBB4_12
LBB4_12:
	b	LBB4_13
LBB4_13:
	b	LBB4_14
LBB4_14:
	ldr	x0, [sp, #16]
	bl	_opt_long_some
	stur	x0, [x29, #-16]
	stur	x1, [x29, #-8]
	b	LBB4_15
LBB4_15:
	ldur	x0, [x29, #-16]
	ldur	x1, [x29, #-8]
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	add	sp, sp, #64
	ret
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function opt_long_some
_opt_long_some:                         ; @opt_long_some
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #32
	.cfi_def_cfa_offset 32
	str	x0, [sp, #8]
	mov	w8, #1                          ; =0x1
	strb	w8, [sp, #16]
	ldr	x8, [sp, #8]
	str	x8, [sp, #24]
	ldr	x0, [sp, #16]
	ldr	x1, [sp, #24]
	add	sp, sp, #32
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_expr                           ; -- Begin function expr
	.p2align	2
_expr:                                  ; @expr
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #144
	stp	x29, x30, [sp, #128]            ; 16-byte Folded Spill
	add	x29, sp, #128
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	stur	x0, [x29, #-16]
	stur	x1, [x29, #-24]
	ldur	x8, [x29, #-16]
	ldr	x8, [x8]
	ldrsb	w8, [x8]
	subs	w8, w8, #34
	cset	w8, ne
	tbnz	w8, #0, LBB6_2
	b	LBB6_1
LBB6_1:
	ldur	x0, [x29, #-24]
	ldur	x1, [x29, #-16]
	bl	_literal_string
	mov	w8, #1                          ; =0x1
	and	w8, w8, #0x1
	and	w8, w8, #0x1
	sturb	w8, [x29, #-1]
	b	LBB6_13
LBB6_2:
	ldur	x0, [x29, #-16]
	bl	_lit_numeric
	stur	x0, [x29, #-40]
	stur	x1, [x29, #-32]
	ldur	x8, [x29, #-32]
	stur	x8, [x29, #-48]
	ldurb	w8, [x29, #-40]
	tbnz	w8, #0, LBB6_4
	b	LBB6_3
LBB6_3:
	mov	w8, #0                          ; =0x0
	and	w8, w8, #0x1
	and	w8, w8, #0x1
	sturb	w8, [x29, #-1]
	b	LBB6_13
LBB6_4:
	ldur	x8, [x29, #-16]
	ldr	x8, [x8, #8]
	ldrb	w8, [x8, #1]
	sturb	w8, [x29, #-49]
	ldursb	w8, [x29, #-49]
	subs	w8, w8, #35
	cset	w8, lt
	tbnz	w8, #0, LBB6_11
	b	LBB6_5
LBB6_5:
	ldursb	w8, [x29, #-49]
	subs	w8, w8, #47
	cset	w8, gt
	tbnz	w8, #0, LBB6_11
	b	LBB6_6
LBB6_6:
	ldur	x8, [x29, #-24]
	add	x1, x8, #32
	add	x0, sp, #56
	bl	_lex
	ldur	x8, [x29, #-24]
	add	x1, x8, #32
	add	x0, sp, #40
	str	x0, [sp, #8]                    ; 8-byte Folded Spill
	bl	_lex
	ldr	x0, [sp, #8]                    ; 8-byte Folded Reload
	bl	_lit_numeric
	str	x0, [sp, #24]
	str	x1, [sp, #32]
	ldr	x8, [sp, #32]
	str	x8, [sp, #16]
	ldrb	w8, [sp, #24]
	tbnz	w8, #0, LBB6_8
	b	LBB6_7
LBB6_7:
	adrp	x0, l_.str.10@PAGE
	add	x0, x0, l_.str.10@PAGEOFF
	bl	_compile_err
	b	LBB6_8
LBB6_8:
	ldr	x8, [sp, #56]
	ldrsb	w8, [x8]
	subs	w8, w8, #43
	cset	w8, ne
	tbnz	w8, #0, LBB6_10
	b	LBB6_9
LBB6_9:
	ldur	x8, [x29, #-24]
	ldr	w0, [x8]
	ldur	x8, [x29, #-24]
	ldr	w1, [x8, #4]
	ldur	x8, [x29, #-48]
	ldr	x9, [sp, #16]
	add	x8, x8, x9
	mov	x2, x8
	bl	_emit_mov
	b	LBB6_10
LBB6_10:
	b	LBB6_12
LBB6_11:
	ldur	x8, [x29, #-24]
	ldr	w0, [x8]
	ldur	x8, [x29, #-24]
	ldr	w1, [x8, #4]
	ldur	x8, [x29, #-48]
	mov	x2, x8
	bl	_emit_mov
	b	LBB6_12
LBB6_12:
	mov	w8, #1                          ; =0x1
	and	w8, w8, #0x1
	and	w8, w8, #0x1
	sturb	w8, [x29, #-1]
	b	LBB6_13
LBB6_13:
	ldurb	w8, [x29, #-1]
	and	w0, w8, #0x1
	ldp	x29, x30, [sp, #128]            ; 16-byte Folded Reload
	add	sp, sp, #144
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_expr_line                      ; -- Begin function expr_line
	.p2align	2
_expr_line:                             ; @expr_line
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #80
	stp	x29, x30, [sp, #64]             ; 16-byte Folded Spill
	add	x29, sp, #64
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	sub	x8, x29, #24
	stur	x0, [x29, #-24]
	stur	x1, [x29, #-16]
	str	x2, [sp, #32]
	str	x8, [sp, #24]
	ldr	x0, [sp, #24]
	ldr	x1, [sp, #32]
	bl	_expr
	mov	w8, #1                          ; =0x1
	and	w8, w0, w8
	strb	w8, [sp, #23]
	ldrb	w8, [sp, #23]
	tbnz	w8, #0, LBB7_2
	b	LBB7_1
LBB7_1:
	mov	w8, #0                          ; =0x0
	and	w8, w8, #0x1
	and	w8, w8, #0x1
	sturb	w8, [x29, #-1]
	b	LBB7_10
LBB7_2:
	b	LBB7_3
LBB7_3:                                 ; =>This Inner Loop Header: Depth=1
	ldr	x8, [sp, #24]
	ldr	x8, [x8, #8]
	ldrsb	w8, [x8]
	subs	w8, w8, #44
	cset	w8, ne
	mov	w9, #0                          ; =0x0
	str	w9, [sp, #16]                   ; 4-byte Folded Spill
	tbnz	w8, #0, LBB7_5
	b	LBB7_4
LBB7_4:                                 ;   in Loop: Header=BB7_3 Depth=1
	ldr	x8, [sp, #24]
	ldr	x8, [x8, #8]
	ldrsb	w0, [x8, #1]
	bl	_isspace
	subs	w8, w0, #0
	cset	w8, ne
	str	w8, [sp, #16]                   ; 4-byte Folded Spill
	b	LBB7_5
LBB7_5:                                 ;   in Loop: Header=BB7_3 Depth=1
	ldr	w8, [sp, #16]                   ; 4-byte Folded Reload
	tbz	w8, #0, LBB7_9
	b	LBB7_6
LBB7_6:                                 ;   in Loop: Header=BB7_3 Depth=1
	ldr	x9, [sp, #32]
	ldr	w8, [x9, #4]
	mov	w10, #1                         ; =0x1
	str	w10, [sp, #12]                  ; 4-byte Folded Spill
	add	w8, w8, #1
	str	w8, [x9, #4]
	ldr	x0, [sp, #24]
	ldr	x8, [sp, #32]
	add	x1, x8, #32
	bl	_lex
	ldr	x0, [sp, #24]
	bl	_str_print
	ldr	x0, [sp, #24]
	ldr	x1, [sp, #32]
	bl	_expr
	ldr	w8, [sp, #12]                   ; 4-byte Folded Reload
	and	w8, w0, w8
	strb	w8, [sp, #23]
	ldrb	w8, [sp, #23]
	tbnz	w8, #0, LBB7_8
	b	LBB7_7
LBB7_7:
	b	LBB7_9
LBB7_8:                                 ;   in Loop: Header=BB7_3 Depth=1
	b	LBB7_3
LBB7_9:
	ldr	x8, [sp, #32]
	str	wzr, [x8, #4]
	mov	w8, #1                          ; =0x1
	and	w8, w8, #0x1
	and	w8, w8, #0x1
	sturb	w8, [x29, #-1]
	b	LBB7_10
LBB7_10:
	ldurb	w8, [x29, #-1]
	and	w0, w8, #0x1
	ldp	x29, x30, [sp, #64]             ; 16-byte Folded Reload
	add	sp, sp, #80
	ret
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function str_print
_str_print:                             ; @str_print
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	str	x0, [sp, #8]
	ldr	x0, [sp, #8]
	adrp	x8, ___stdoutp@GOTPAGE
	ldr	x8, [x8, ___stdoutp@GOTPAGEOFF]
	str	x8, [sp]                        ; 8-byte Folded Spill
	ldr	x1, [x8]
	bl	_str_fprintnl
	ldr	x8, [sp]                        ; 8-byte Folded Reload
	ldr	x1, [x8]
	mov	w0, #10                         ; =0xa
	bl	_fputc
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #32
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_parse                          ; -- Begin function parse
	.p2align	2
_parse:                                 ; @parse
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #96
	stp	x29, x30, [sp, #80]             ; 16-byte Folded Spill
	add	x29, sp, #80
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	stur	x0, [x29, #-8]
	stur	x1, [x29, #-16]
	ldur	x8, [x29, #-8]
	ldur	x2, [x29, #-16]
	ldr	x0, [x8]
	ldr	x1, [x8, #8]
	bl	_expr_line
	tbz	w0, #0, LBB9_2
	b	LBB9_1
LBB9_1:
	b	LBB9_25
LBB9_2:
	ldur	x0, [x29, #-8]
	adrp	x1, l_.str.11@PAGE
	add	x1, x1, l_.str.11@PAGEOFF
	bl	_str_eq_lit
	tbz	w0, #0, LBB9_4
	b	LBB9_3
LBB9_3:
	ldur	x8, [x29, #-16]
	str	wzr, [x8]
	b	LBB9_24
LBB9_4:
	ldur	x0, [x29, #-8]
	adrp	x1, l_.str.12@PAGE
	add	x1, x1, l_.str.12@PAGEOFF
	bl	_str_ends_with
	tbz	w0, #0, LBB9_13
	b	LBB9_5
LBB9_5:
	ldur	x8, [x29, #-8]
	ldr	x9, [x8]
	add	x8, sp, #40
	str	x9, [sp, #40]
	ldur	x9, [x29, #-8]
	ldr	x9, [x9, #8]
	subs	x9, x9, #2
	str	x9, [sp, #48]
	stur	x8, [x29, #-24]
	ldur	x8, [x29, #-16]
	add	x0, x8, #16
	bl	_str_is_empty
	tbnz	w0, #0, LBB9_8
	b	LBB9_6
LBB9_6:
	ldur	x0, [x29, #-24]
	bl	_str_is_empty
	tbz	w0, #0, LBB9_8
	b	LBB9_7
LBB9_7:
	ldur	x8, [x29, #-16]
	add	x0, x8, #16
	bl	_str_move
	mov	x8, x0
	add	x0, sp, #24
	str	x8, [sp, #24]
	str	x1, [sp, #32]
	bl	_emit_fn_call
	b	LBB9_12
LBB9_8:
	ldur	x0, [x29, #-24]
	bl	_str_is_empty
	tbnz	w0, #0, LBB9_10
	b	LBB9_9
LBB9_9:
	ldur	x0, [x29, #-24]
	bl	_emit_fn_call
	b	LBB9_11
LBB9_10:
	adrp	x0, l_.str.13@PAGE
	add	x0, x0, l_.str.13@PAGEOFF
	bl	_compile_err
	b	LBB9_11
LBB9_11:
	b	LBB9_12
LBB9_12:
	ldur	x8, [x29, #-16]
	str	wzr, [x8, #4]
	ldur	x9, [x29, #-16]
	mov	w8, #2                          ; =0x2
	str	w8, [x9]
	ldur	x8, [x29, #-16]
	strb	wzr, [x8, #56]
	b	LBB9_23
LBB9_13:
	ldur	x8, [x29, #-8]
	ldr	x8, [x8]
	ldrsb	w0, [x8]
	bl	_islower
	subs	w8, w0, #0
	cset	w8, ne
	tbnz	w8, #0, LBB9_15
	b	LBB9_14
LBB9_14:
	ldur	x8, [x29, #-8]
	ldr	x8, [x8]
	ldrsb	w8, [x8]
	subs	w8, w8, #95
	cset	w8, ne
	tbnz	w8, #0, LBB9_16
	b	LBB9_15
LBB9_15:
	ldur	x9, [x29, #-16]
	mov	w8, #1                          ; =0x1
	str	w8, [x9]
	ldur	x8, [x29, #-16]
	ldur	x9, [x29, #-8]
	ldr	q0, [x9]
	str	q0, [x8, #16]
	b	LBB9_22
LBB9_16:
	ldur	x8, [x29, #-8]
	ldr	x8, [x8]
	ldrsb	w0, [x8]
	bl	_isupper
	subs	w8, w0, #0
	cset	w8, eq
	tbnz	w8, #0, LBB9_20
	b	LBB9_17
LBB9_17:
	ldur	x8, [x29, #-16]
	add	x1, x8, #32
	add	x0, sp, #8
	str	x0, [sp]                        ; 8-byte Folded Spill
	bl	_lex
	ldr	x0, [sp]                        ; 8-byte Folded Reload
	adrp	x1, l_.str.14@PAGE
	add	x1, x1, l_.str.14@PAGEOFF
	bl	_str_eq_lit
	tbz	w0, #0, LBB9_19
	b	LBB9_18
LBB9_18:
	adrp	x0, l_.str.15@PAGE
	add	x0, x0, l_.str.15@PAGEOFF
	bl	_printf
	ldur	x9, [x29, #-16]
	mov	w8, #3                          ; =0x3
	str	w8, [x9]
	ldur	x10, [x29, #-16]
	ldr	w8, [x10, #8]
	add	w9, w8, #1
	str	w9, [x10, #8]
	ldur	x9, [x29, #-16]
	str	w8, [x9, #4]
	b	LBB9_19
LBB9_19:
	b	LBB9_21
LBB9_20:
	adrp	x0, l_.str.16@PAGE
	add	x0, x0, l_.str.16@PAGEOFF
	bl	_compile_err
	ldur	x0, [x29, #-8]
	adrp	x8, ___stderrp@GOTPAGE
	ldr	x8, [x8, ___stderrp@GOTPAGEOFF]
	ldr	x1, [x8]
	bl	_str_fprint
	b	LBB9_21
LBB9_21:
	b	LBB9_22
LBB9_22:
	b	LBB9_23
LBB9_23:
	b	LBB9_24
LBB9_24:
	b	LBB9_25
LBB9_25:
	ldp	x29, x30, [sp, #80]             ; 16-byte Folded Reload
	add	sp, sp, #96
	ret
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function str_ends_with
_str_ends_with:                         ; @str_ends_with
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
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
	ldur	x0, [x29, #-8]
	bl	_str_len
	ldr	x8, [sp, #8]
	subs	x8, x0, x8
	cset	w8, lo
	mov	w9, #0                          ; =0x0
	str	w9, [sp, #4]                    ; 4-byte Folded Spill
	tbnz	w8, #0, LBB10_2
	b	LBB10_1
LBB10_1:
	ldur	x8, [x29, #-8]
	ldr	x8, [x8, #8]
	ldr	x9, [sp, #8]
	subs	x0, x8, x9
	ldr	x1, [sp, #16]
	ldr	x2, [sp, #8]
	bl	_memcmp
	subs	w8, w0, #0
	cset	w8, eq
	str	w8, [sp, #4]                    ; 4-byte Folded Spill
	b	LBB10_2
LBB10_2:
	ldr	w8, [sp, #4]                    ; 4-byte Folded Reload
	and	w0, w8, #0x1
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function str_move
_str_move:                              ; @str_move
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #32
	.cfi_def_cfa_offset 32
	str	x0, [sp, #8]
	ldr	x8, [sp, #8]
	ldr	q0, [x8]
	str	q0, [sp, #16]
	ldr	x8, [sp, #8]
	adrp	x9, _str_null@PAGE
	add	x9, x9, _str_null@PAGEOFF
	ldr	q0, [x9]
	str	q0, [x8]
	ldr	x0, [sp, #16]
	ldr	x1, [sp, #24]
	add	sp, sp, #32
	ret
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function str_fprint
_str_fprint:                            ; @str_fprint
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	str	x0, [sp, #8]
	str	x1, [sp]
	ldr	x0, [sp, #8]
	ldr	x1, [sp]
	bl	_str_fprintnl
	ldr	x1, [sp]
	mov	w0, #10                         ; =0xa
	bl	_fputc
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
	sub	sp, sp, #272
	stp	x28, x27, [sp, #240]            ; 16-byte Folded Spill
	stp	x29, x30, [sp, #256]            ; 16-byte Folded Spill
	add	x29, sp, #256
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w27, -24
	.cfi_offset w28, -32
	stur	wzr, [x29, #-20]
	stur	w0, [x29, #-24]
	stur	x1, [x29, #-32]
	ldur	x8, [x29, #-32]
	ldr	x8, [x8, #8]
	stur	x8, [x29, #-40]
	ldur	x8, [x29, #-32]
	ldr	x0, [x8, #8]
	adrp	x1, l_.str.17@PAGE
	add	x1, x1, l_.str.17@PAGEOFF
	bl	_fopen
	stur	x0, [x29, #-48]
	ldur	x8, [x29, #-48]
	subs	x8, x8, #0
	cset	w8, ne
	tbnz	w8, #0, LBB13_2
	b	LBB13_1
LBB13_1:
	adrp	x8, ___stderrp@GOTPAGE
	ldr	x8, [x8, ___stderrp@GOTPAGEOFF]
	ldr	x0, [x8]
	ldur	x8, [x29, #-32]
	ldr	x8, [x8, #8]
	mov	x9, sp
	str	x8, [x9]
	adrp	x1, l_.str.18@PAGE
	add	x1, x1, l_.str.18@PAGEOFF
	bl	_fprintf
	mov	w0, #1                          ; =0x1
	bl	_exit
LBB13_2:
	ldur	x0, [x29, #-48]
	mov	x1, #0                          ; =0x0
	mov	w2, #2                          ; =0x2
	bl	_fseek
	ldur	x0, [x29, #-48]
	bl	_ftell
	stur	x0, [x29, #-56]
	ldur	x0, [x29, #-48]
	bl	_rewind
	ldur	x0, [x29, #-56]
	bl	_malloc
	stur	x0, [x29, #-64]
	ldur	x0, [x29, #-64]
	ldur	x2, [x29, #-56]
	mov	w1, #0                          ; =0x0
	mov	x3, #-1                         ; =0xffffffffffffffff
	bl	___memset_chk
	ldur	x8, [x29, #-64]
	subs	x8, x8, #0
	cset	w8, ne
	tbnz	w8, #0, LBB13_4
	b	LBB13_3
LBB13_3:
	adrp	x8, ___stderrp@GOTPAGE
	ldr	x8, [x8, ___stderrp@GOTPAGEOFF]
	ldr	x1, [x8]
	adrp	x0, l_.str.19@PAGE
	add	x0, x0, l_.str.19@PAGEOFF
	bl	_fputs
	mov	w0, #1                          ; =0x1
	bl	_exit
LBB13_4:
	ldur	x0, [x29, #-64]
	ldur	x2, [x29, #-56]
	ldur	x3, [x29, #-48]
	mov	x1, #1                          ; =0x1
	bl	_fread
	stur	x0, [x29, #-72]
	ldur	x8, [x29, #-72]
	ldur	x9, [x29, #-56]
	subs	x8, x8, x9
	cset	w8, ls
	tbnz	w8, #0, LBB13_6
	b	LBB13_5
LBB13_5:
	adrp	x8, ___stderrp@GOTPAGE
	ldr	x8, [x8, ___stderrp@GOTPAGEOFF]
	ldr	x0, [x8]
	ldur	x10, [x29, #-56]
	ldur	x8, [x29, #-72]
	mov	x9, sp
	str	x10, [x9]
	str	x8, [x9, #8]
	adrp	x1, l_.str.20@PAGE
	add	x1, x1, l_.str.20@PAGEOFF
	bl	_fprintf
	mov	w0, #1                          ; =0x1
	bl	_exit
LBB13_6:
	ldur	x8, [x29, #-64]
	stur	x8, [x29, #-96]
	ldur	x8, [x29, #-64]
	stur	x8, [x29, #-88]
	ldur	x8, [x29, #-64]
	ldur	x9, [x29, #-56]
	add	x8, x8, x9
	stur	x8, [x29, #-80]
	bl	_emit_init
	bl	_emit_mainfn
	add	x8, sp, #88
	mov	w9, #2                          ; =0x2
	str	w9, [sp, #88]
	str	wzr, [sp, #92]
	str	wzr, [sp, #96]
	str	xzr, [sp, #104]
	str	xzr, [sp, #112]
	ldur	q0, [x29, #-96]
	stur	q0, [sp, #120]
	ldur	x9, [x29, #-80]
	str	x9, [sp, #136]
	mov	w9, #1                          ; =0x1
	strb	w9, [sp, #144]
	stur	x8, [x29, #-104]
	ldur	x8, [x29, #-104]
	add	x8, x8, #32
	str	x8, [sp, #80]
	add	x8, sp, #56
                                        ; kill: def $x9 killed $xzr
	str	xzr, [sp, #56]
	str	xzr, [sp, #64]
	str	x8, [sp, #72]
	b	LBB13_7
LBB13_7:                                ; =>This Inner Loop Header: Depth=1
	ldr	x8, [sp, #80]
	ldr	x8, [x8]
	ldr	x9, [sp, #80]
	ldr	x9, [x9, #16]
	subs	x8, x8, x9
	cset	w8, hs
	tbnz	w8, #0, LBB13_11
	b	LBB13_8
LBB13_8:                                ;   in Loop: Header=BB13_7 Depth=1
	ldr	x0, [sp, #72]
	ldr	x1, [sp, #80]
	bl	_lex
	ldr	x0, [sp, #72]
	bl	_str_len
	subs	x8, x0, #0
	cset	w8, ne
	tbnz	w8, #0, LBB13_10
	b	LBB13_9
LBB13_9:                                ;   in Loop: Header=BB13_7 Depth=1
	b	LBB13_7
LBB13_10:                               ;   in Loop: Header=BB13_7 Depth=1
	ldr	x0, [sp, #72]
	bl	_str_print
	ldr	x0, [sp, #72]
	ldur	x1, [x29, #-104]
	bl	_parse
	b	LBB13_7
LBB13_11:
	ldur	x0, [x29, #-104]
	bl	_emit_fn_prologue
	ldur	x0, [x29, #-104]
	bl	_emit_fn_epilogue
	bl	_emit_ret
	ldur	x0, [x29, #-40]
	bl	_strlen
	str	x0, [sp, #48]
	ldr	x8, [sp, #48]
	add	x0, x8, #1
	bl	_malloc
	str	x0, [sp, #40]
	ldr	x0, [sp, #40]
	ldr	x8, [sp, #48]
	add	x2, x8, #1
	mov	w1, #0                          ; =0x0
	mov	x3, #-1                         ; =0xffffffffffffffff
	str	x3, [sp, #24]                   ; 8-byte Folded Spill
	bl	___memset_chk
	ldr	x3, [sp, #24]                   ; 8-byte Folded Reload
	ldr	x0, [sp, #40]
	ldur	x1, [x29, #-40]
	ldr	x8, [sp, #48]
	subs	x2, x8, #1
	bl	___strncpy_chk
	ldr	x8, [sp, #40]
	ldr	x9, [sp, #48]
	subs	x9, x9, #2
	add	x9, x8, x9
	mov	w8, #115                        ; =0x73
	strb	w8, [x9]
	ldr	x0, [sp, #40]
	adrp	x1, l_.str.21@PAGE
	add	x1, x1, l_.str.21@PAGEOFF
	bl	_fopen
	str	x0, [sp, #32]
	ldr	x8, [sp, #32]
	subs	x8, x8, #0
	cset	w8, ne
	tbnz	w8, #0, LBB13_13
	b	LBB13_12
LBB13_12:
	adrp	x8, ___stderrp@GOTPAGE
	ldr	x8, [x8, ___stderrp@GOTPAGEOFF]
	ldr	x0, [x8]
	adrp	x1, l_.str.22@PAGE
	add	x1, x1, l_.str.22@PAGEOFF
	bl	_fprintf
	mov	w0, #1                          ; =0x1
	bl	_exit
LBB13_13:
	ldr	x0, [sp, #32]
	bl	_emit
	adrp	x8, _has_compile_err@PAGE
	ldrb	w8, [x8, _has_compile_err@PAGEOFF]
	and	w0, w8, #0x1
	ldp	x29, x30, [sp, #256]            ; 16-byte Folded Reload
	ldp	x28, x27, [sp, #240]            ; 16-byte Folded Reload
	add	sp, sp, #272
	ret
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function str_fprintnl
_str_fprintnl:                          ; @str_fprintnl
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	stur	x0, [x29, #-8]
	str	x1, [sp, #16]
	ldur	x8, [x29, #-8]
	ldr	x8, [x8]
	ldur	x9, [x29, #-8]
	ldr	x9, [x9, #8]
	subs	x8, x8, x9
	cset	w8, ne
	tbnz	w8, #0, LBB14_2
	b	LBB14_1
LBB14_1:
	ldr	x1, [sp, #16]
	adrp	x0, l_.str.23@PAGE
	add	x0, x0, l_.str.23@PAGEOFF
	bl	_fputs
	b	LBB14_3
LBB14_2:
	ldur	x8, [x29, #-8]
	ldr	x8, [x8]
	str	x8, [sp, #8]                    ; 8-byte Folded Spill
	ldur	x0, [x29, #-8]
	bl	_str_len
	mov	x2, x0
	ldr	x0, [sp, #8]                    ; 8-byte Folded Reload
	ldr	x3, [sp, #16]
	mov	x1, #1                          ; =0x1
	bl	_fwrite
	b	LBB14_3
LBB14_3:
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
	.cfi_endproc
                                        ; -- End function
	.section	__DATA,__data
	.globl	_lineno                         ; @lineno
	.p2align	2, 0x0
_lineno:
	.long	1                               ; 0x1

	.globl	_has_compile_err                ; @has_compile_err
.zerofill __DATA,__common,_has_compile_err,1,0
	.section	__TEXT,__const
	.p2align	3, 0x0                          ; @str_null
_str_null:
	.space	16

	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"\033[31m"

l_.str.1:                               ; @.str.1
	.asciz	"error in line %d: "

l_.str.2:                               ; @.str.2
	.asciz	"\033[0m"

l_.str.3:                               ; @.str.3
	.asciz	"\033[33m"

l_.str.4:                               ; @.str.4
	.asciz	"warning in line %d: "

l_.str.5:                               ; @.str.5
	.asciz	"expected closing \"\n"

l_.str.6:                               ; @.str.6
	.asciz	"%d is too large for a string literal"

l_.str.7:                               ; @.str.7
	.asciz	"expected closing '\n"

l_.str.8:                               ; @.str.8
	.asciz	"true"

l_.str.9:                               ; @.str.9
	.asciz	"false"

	.section	__TEXT,__const
	.p2align	3, 0x0                          ; @opt_long_none
_opt_long_none:
	.space	16

	.section	__TEXT,__cstring,cstring_literals
l_.str.10:                              ; @.str.10
	.asciz	"expected operand\n"

l_.str.11:                              ; @.str.11
	.asciz	"ret"

l_.str.12:                              ; @.str.12
	.asciz	"=>"

l_.str.13:                              ; @.str.13
	.asciz	"empty function name"

l_.str.14:                              ; @.str.14
	.asciz	"::"

l_.str.15:                              ; @.str.15
	.asciz	"declare nreg statement"

l_.str.16:                              ; @.str.16
	.asciz	"unknown token "

l_.str.17:                              ; @.str.17
	.asciz	"r"

l_.str.18:                              ; @.str.18
	.asciz	"error: could not open file %s\n"

l_.str.19:                              ; @.str.19
	.asciz	"error: malloc failed\n"

l_.str.20:                              ; @.str.20
	.asciz	"error: buffer overflow. expected %ld bytes but read %lu bytes\n"

l_.str.21:                              ; @.str.21
	.asciz	"w"

l_.str.22:                              ; @.str.22
	.asciz	"error: failed to create file\n"

l_.str.23:                              ; @.str.23
	.asciz	"(empty)"

.subsections_via_symbols
