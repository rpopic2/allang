	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 14, 0	sdk_version 15, 2
	.globl	_lex                            ; -- Begin function lex
	.p2align	2
_lex:                                   ; @lex
	.cfi_startproc
; %bb.0:
	adrp	x8, _lineno@PAGE
	ldr	w9, [x8, _lineno@PAGEOFF]
	ldr	x14, [x1]
	mov	w10, #1                         ; =0x1
	mov	x11, #1025                      ; =0x401
	movk	x11, #4097, lsl #32
LBB0_1:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_4 Depth 2
                                        ;       Child Loop BB0_9 Depth 3
                                        ;     Child Loop BB0_15 Depth 2
	stp	x14, xzr, [x0]
	ldr	x13, [x1]
	mov	x12, x14
	b	LBB0_4
LBB0_2:                                 ;   in Loop: Header=BB0_4 Depth=2
	mov	w14, w15
	cmp	w15, #44
	lsl	x14, x10, x14
	and	x14, x14, x11
	ccmp	x14, #0, #4, ls
	b.ne	LBB0_12
LBB0_3:                                 ;   in Loop: Header=BB0_4 Depth=2
	add	x13, x13, #1
	str	x13, [x1]
LBB0_4:                                 ;   Parent Loop BB0_1 Depth=1
                                        ; =>  This Loop Header: Depth=2
                                        ;       Child Loop BB0_9 Depth 3
	ldrb	w14, [x13]
	cmp	w14, #47
	b.eq	LBB0_7
; %bb.5:                                ;   in Loop: Header=BB0_4 Depth=2
	cmp	w14, #34
	b.eq	LBB0_14
; %bb.6:                                ;   in Loop: Header=BB0_4 Depth=2
	mov	x15, x14
	cmp	w14, #10
	b.ne	LBB0_2
	b	LBB0_11
LBB0_7:                                 ;   in Loop: Header=BB0_4 Depth=2
	ldrb	w14, [x13, #1]
	cmp	w14, #47
	b.ne	LBB0_3
; %bb.8:                                ;   in Loop: Header=BB0_4 Depth=2
	add	x13, x13, #1
LBB0_9:                                 ;   Parent Loop BB0_1 Depth=1
                                        ;     Parent Loop BB0_4 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	str	x13, [x1]
	ldrb	w12, [x13], #1
	cmp	w12, #10
	b.ne	LBB0_9
; %bb.10:                               ;   in Loop: Header=BB0_4 Depth=2
	sub	x12, x13, #1
	str	x12, [x0]
	ldurb	w14, [x13, #-1]
	mov	w15, #10                        ; =0xa
	mov	x13, x12
	cmp	w14, #10
	b.ne	LBB0_2
LBB0_11:                                ;   in Loop: Header=BB0_4 Depth=2
	add	w9, w9, #1
	str	w9, [x8, _lineno@PAGEOFF]
	b	LBB0_2
LBB0_12:                                ;   in Loop: Header=BB0_1 Depth=1
	add	x14, x13, #1
	str	x14, [x1]
	str	x13, [x0, #8]
	cmp	w15, #44
	b.ne	LBB0_17
; %bb.13:                               ;   in Loop: Header=BB0_1 Depth=1
	add	x14, x13, #2
	str	x14, [x1]
	b	LBB0_17
LBB0_14:                                ;   in Loop: Header=BB0_1 Depth=1
	add	x13, x13, #1
LBB0_15:                                ;   Parent Loop BB0_1 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	str	x13, [x1]
	ldrb	w14, [x13], #1
	cmp	w14, #34
	ccmp	w14, #10, #4, ne
	b.ne	LBB0_15
; %bb.16:                               ;   in Loop: Header=BB0_1 Depth=1
	str	x13, [x1]
	str	x13, [x0, #8]
	mov	x14, x13
LBB0_17:                                ;   in Loop: Header=BB0_1 Depth=1
	ldr	x15, [x1, #16]
	cmp	x13, x15
	b.hi	LBB0_20
; %bb.18:                               ;   in Loop: Header=BB0_1 Depth=1
	cmp	x13, x12
	b.eq	LBB0_1
; %bb.19:
	ret
LBB0_20:
	stp	xzr, xzr, [x0]
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_compile_err                    ; -- Begin function compile_err
	.p2align	2
_compile_err:                           ; @compile_err
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	stp	x20, x19, [sp, #16]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	mov	x19, x0
	mov	w8, #1                          ; =0x1
	adrp	x9, _has_compile_err@PAGE
	strb	w8, [x9, _has_compile_err@PAGEOFF]
Lloh0:
	adrp	x20, ___stderrp@GOTPAGE
Lloh1:
	ldr	x20, [x20, ___stderrp@GOTPAGEOFF]
	ldr	x1, [x20]
Lloh2:
	adrp	x0, l_.str@PAGE
Lloh3:
	add	x0, x0, l_.str@PAGEOFF
	bl	_fputs
	ldr	x0, [x20]
Lloh4:
	adrp	x8, _lineno@PAGE
Lloh5:
	ldr	w8, [x8, _lineno@PAGEOFF]
	str	x8, [sp]
Lloh6:
	adrp	x1, l_.str.1@PAGE
Lloh7:
	add	x1, x1, l_.str.1@PAGEOFF
	bl	_fprintf
	add	x8, x29, #16
	str	x8, [sp, #8]
	ldr	x0, [x20]
	add	x2, x29, #16
	mov	x1, x19
	bl	_vfprintf
	ldr	x1, [x20]
Lloh8:
	adrp	x0, l_.str.2@PAGE
Lloh9:
	add	x0, x0, l_.str.2@PAGEOFF
	bl	_fputs
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
	.loh AdrpAdd	Lloh8, Lloh9
	.loh AdrpAdd	Lloh6, Lloh7
	.loh AdrpLdr	Lloh4, Lloh5
	.loh AdrpAdd	Lloh2, Lloh3
	.loh AdrpLdrGot	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.globl	_compile_warning                ; -- Begin function compile_warning
	.p2align	2
_compile_warning:                       ; @compile_warning
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	stp	x20, x19, [sp, #16]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	mov	x19, x0
Lloh10:
	adrp	x20, ___stderrp@GOTPAGE
Lloh11:
	ldr	x20, [x20, ___stderrp@GOTPAGEOFF]
	ldr	x1, [x20]
Lloh12:
	adrp	x0, l_.str.3@PAGE
Lloh13:
	add	x0, x0, l_.str.3@PAGEOFF
	bl	_fputs
	ldr	x0, [x20]
Lloh14:
	adrp	x8, _lineno@PAGE
Lloh15:
	ldr	w8, [x8, _lineno@PAGEOFF]
	str	x8, [sp]
Lloh16:
	adrp	x1, l_.str.4@PAGE
Lloh17:
	add	x1, x1, l_.str.4@PAGEOFF
	bl	_fprintf
	add	x8, x29, #16
	str	x8, [sp, #8]
	ldr	x0, [x20]
	add	x2, x29, #16
	mov	x1, x19
	bl	_vfprintf
	ldr	x1, [x20]
Lloh18:
	adrp	x0, l_.str.2@PAGE
Lloh19:
	add	x0, x0, l_.str.2@PAGEOFF
	bl	_fputs
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
	.loh AdrpAdd	Lloh18, Lloh19
	.loh AdrpAdd	Lloh16, Lloh17
	.loh AdrpLdr	Lloh14, Lloh15
	.loh AdrpAdd	Lloh12, Lloh13
	.loh AdrpLdrGot	Lloh10, Lloh11
	.cfi_endproc
                                        ; -- End function
	.globl	_literal_string                 ; -- Begin function literal_string
	.p2align	2
_literal_string:                        ; @literal_string
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #112
	stp	x26, x25, [sp, #32]             ; 16-byte Folded Spill
	stp	x24, x23, [sp, #48]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #64]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #80]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #96]             ; 16-byte Folded Spill
	add	x29, sp, #96
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	.cfi_offset w25, -72
	.cfi_offset w26, -80
	mov	x20, x1
	mov	x19, x0
	bl	_emit_need_escaping
	mov	x21, x0
	ldr	x22, [x20, #8]
	ldurb	w8, [x22, #-1]
	cmp	w8, #34
	b.eq	LBB3_2
; %bb.1:
Lloh20:
	adrp	x0, l_.str.5@PAGE
Lloh21:
	add	x0, x0, l_.str.5@PAGEOFF
	bl	_compile_err
LBB3_2:
	tbz	w21, #0, LBB3_18
; %bb.3:
	ldr	x21, [x20]
	subs	x0, x22, x21
	bl	_malloc
	mov	x20, x0
	subs	x24, x22, x21
	add	x23, x0, x24
	b.eq	LBB3_19
; %bb.4:
	mov	x8, #0                          ; =0x0
	mov	w25, #0                         ; =0x0
	mov	x26, x20
Lloh22:
	adrp	x22, l_.str.6@PAGE
Lloh23:
	add	x22, x22, l_.str.6@PAGEOFF
	b	LBB3_7
LBB3_5:                                 ;   in Loop: Header=BB3_7 Depth=1
	mov	w0, #0                          ; =0x0
LBB3_6:                                 ;   in Loop: Header=BB3_7 Depth=1
	strb	w0, [x26], #1
	add	w25, w25, #1
	sxtw	x8, w25
	cmp	x24, x8
	b.ls	LBB3_19
LBB3_7:                                 ; =>This Inner Loop Header: Depth=1
	ldrb	w0, [x21, x8]
	cmp	w0, #92
	b.ne	LBB3_6
; %bb.8:                                ;   in Loop: Header=BB3_7 Depth=1
                                        ; kill: def $w25 killed $w25 killed $x25 def $x25
	sxtw	x8, w25
	add	x25, x8, #1
	ldrsb	w8, [x21, x25]
                                        ; kill: def $w25 killed $w25 killed $x25 def $x25
	cmp	w8, #109
	b.gt	LBB3_12
; %bb.9:                                ;   in Loop: Header=BB3_7 Depth=1
	cmp	w8, #48
	b.eq	LBB3_5
; %bb.10:                               ;   in Loop: Header=BB3_7 Depth=1
	cmp	w8, #92
	b.ne	LBB3_16
; %bb.11:                               ;   in Loop: Header=BB3_7 Depth=1
	mov	w0, #92                         ; =0x5c
	b	LBB3_6
LBB3_12:                                ;   in Loop: Header=BB3_7 Depth=1
	cmp	w8, #110
	b.eq	LBB3_15
; %bb.13:                               ;   in Loop: Header=BB3_7 Depth=1
	cmp	w8, #116
	b.ne	LBB3_16
; %bb.14:                               ;   in Loop: Header=BB3_7 Depth=1
	mov	w0, #9                          ; =0x9
	b	LBB3_6
LBB3_15:                                ;   in Loop: Header=BB3_7 Depth=1
	mov	w0, #10                         ; =0xa
	b	LBB3_6
LBB3_16:                                ;   in Loop: Header=BB3_7 Depth=1
	mov	x0, x21
	mov	x1, #0                          ; =0x0
	mov	w2, #0                          ; =0x0
	bl	_strtol
	cmp	x0, #2
	b.lo	LBB3_6
; %bb.17:                               ;   in Loop: Header=BB3_7 Depth=1
	str	x0, [sp]
	mov	x0, x22
	bl	_compile_err
	mov	w0, #32                         ; =0x20
	b	LBB3_6
LBB3_18:
	ldp	w0, w1, [x19]
	mov	x2, x20
	ldp	x29, x30, [sp, #96]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #80]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #64]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #48]             ; 16-byte Folded Reload
	ldp	x26, x25, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #112
	b	_emit_string_lit
LBB3_19:
	stp	x20, x23, [sp, #16]
	ldp	w0, w1, [x19]
	add	x2, sp, #16
	bl	_emit_string_lit
	mov	x0, x20
	bl	_free
	ldp	x29, x30, [sp, #96]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #80]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #64]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #48]             ; 16-byte Folded Reload
	ldp	x26, x25, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #112
	ret
	.loh AdrpAdd	Lloh20, Lloh21
	.loh AdrpAdd	Lloh22, Lloh23
	.cfi_endproc
                                        ; -- End function
	.globl	_lit_numeric                    ; -- Begin function lit_numeric
	.p2align	2
_lit_numeric:                           ; @lit_numeric
	.cfi_startproc
; %bb.0:
	stp	x20, x19, [sp, #-32]!           ; 16-byte Folded Spill
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	ldr	x19, [x0]
	ldrsb	w8, [x19]
	sub	w9, w8, #48
	cmp	w9, #9
	b.hi	LBB4_4
; %bb.1:
	mov	x0, x19
	mov	x1, #0                          ; =0x0
	mov	w2, #0                          ; =0x0
	bl	_strtol
	mov	x19, x0
LBB4_2:
	mov	w0, #1                          ; =0x1
LBB4_3:
	mov	x1, x19
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp], #32             ; 16-byte Folded Reload
	ret
LBB4_4:
	and	w8, w8, #0xff
	cmp	w8, #39
	b.ne	LBB4_7
; %bb.5:
	ldrsb	x19, [x19, #1]
	ldr	x8, [x0, #8]
	ldurb	w8, [x8, #-1]
	cmp	w8, #39
	b.eq	LBB4_2
; %bb.6:
Lloh24:
	adrp	x0, l_.str.7@PAGE
Lloh25:
	add	x0, x0, l_.str.7@PAGEOFF
	bl	_compile_err
	b	LBB4_2
LBB4_7:
	ldr	x8, [x0, #8]
	sub	x20, x8, x19
Lloh26:
	adrp	x1, l_.str.8@PAGE
Lloh27:
	add	x1, x1, l_.str.8@PAGEOFF
	mov	x0, x19
	mov	x2, x20
	bl	_memcmp
	cbz	w0, LBB4_9
; %bb.8:
Lloh28:
	adrp	x1, l_.str.9@PAGE
Lloh29:
	add	x1, x1, l_.str.9@PAGEOFF
	mov	x0, x19
	mov	x2, x20
	bl	_memcmp
	mov	x8, x0
	mov	x19, #0                         ; =0x0
	mov	x0, #0                          ; =0x0
	cbnz	w8, LBB4_3
	b	LBB4_2
LBB4_9:
	mov	w19, #1                         ; =0x1
	b	LBB4_2
	.loh AdrpAdd	Lloh24, Lloh25
	.loh AdrpAdd	Lloh26, Lloh27
	.loh AdrpAdd	Lloh28, Lloh29
	.cfi_endproc
                                        ; -- End function
	.globl	_expr                           ; -- Begin function expr
	.p2align	2
_expr:                                  ; @expr
	.cfi_startproc
; %bb.0:
	stp	x24, x23, [sp, #-64]!           ; 16-byte Folded Spill
	stp	x22, x21, [sp, #16]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #32]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	mov	x19, x1
	mov	x21, x0
	ldr	x8, [x0]
	ldrb	w8, [x8]
	cmp	w8, #34
	b.ne	LBB5_2
; %bb.1:
	mov	x0, x19
	mov	x1, x21
	bl	_literal_string
	b	LBB5_53
LBB5_2:
	mov	x0, x21
	bl	_lit_numeric
	tbnz	w0, #0, LBB5_4
; %bb.3:
	mov	w0, #0                          ; =0x0
	b	LBB5_54
LBB5_4:
	mov	x20, x1
	ldr	x8, [x21, #8]
	ldrb	w8, [x8, #1]
	sub	w8, w8, #35
	cmp	w8, #12
	b.hi	LBB5_22
; %bb.5:
	adrp	x8, _lineno@PAGE
	ldr	w9, [x8, _lineno@PAGEOFF]
	ldr	x10, [x19, #32]
Lloh30:
	adrp	x11, lJTI5_0@PAGE
Lloh31:
	add	x11, x11, lJTI5_0@PAGEOFF
	ldr	x12, [x19, #48]
LBB5_6:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB5_8 Depth 2
                                        ;     Child Loop BB5_11 Depth 2
                                        ;     Child Loop BB5_13 Depth 2
	add	x13, x10, #1
	b	LBB5_8
LBB5_7:                                 ;   in Loop: Header=BB5_8 Depth=2
	str	x13, [x19, #32]
	add	x13, x13, #1
LBB5_8:                                 ;   Parent Loop BB5_6 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldurb	w15, [x13, #-1]
	cmp	w15, #47
	b.hi	LBB5_7
; %bb.9:                                ;   in Loop: Header=BB5_8 Depth=2
	adr	x14, LBB5_7
	ldrb	w16, [x11, x15]
	add	x14, x14, x16, lsl #2
	br	x14
LBB5_10:                                ;   in Loop: Header=BB5_8 Depth=2
	ldrb	w14, [x13]
	cmp	w14, #47
	b.ne	LBB5_7
LBB5_11:                                ;   Parent Loop BB5_6 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	str	x13, [x19, #32]
	ldrb	w10, [x13], #1
	cmp	w10, #10
	b.ne	LBB5_11
; %bb.12:                               ;   in Loop: Header=BB5_6 Depth=1
	sub	x14, x13, #1
	mov	x10, x14
	b	LBB5_18
LBB5_13:                                ;   Parent Loop BB5_6 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	str	x13, [x19, #32]
	ldrb	w14, [x13], #1
	cmp	w14, #34
	ccmp	w14, #10, #4, ne
	b.ne	LBB5_13
; %bb.14:                               ;   in Loop: Header=BB5_6 Depth=1
	mov	x14, x13
	b	LBB5_19
LBB5_15:                                ;   in Loop: Header=BB5_6 Depth=1
	str	x13, [x19, #32]
	sub	x14, x13, #1
	cmp	w15, #44
	b.ne	LBB5_20
; %bb.16:                               ;   in Loop: Header=BB5_6 Depth=1
	add	x13, x13, #1
	b	LBB5_19
LBB5_17:                                ;   in Loop: Header=BB5_6 Depth=1
	sub	x14, x13, #1
LBB5_18:                                ;   in Loop: Header=BB5_6 Depth=1
	add	w9, w9, #1
	str	w9, [x8, _lineno@PAGEOFF]
	add	x13, x14, #1
LBB5_19:                                ;   in Loop: Header=BB5_6 Depth=1
	str	x13, [x19, #32]
LBB5_20:                                ;   in Loop: Header=BB5_6 Depth=1
	mov	x23, x10
	mov	x10, x13
	cmp	x14, x12
	b.hi	LBB5_23
; %bb.21:                               ;   in Loop: Header=BB5_6 Depth=1
	cmp	x14, x23
	b.eq	LBB5_6
	b	LBB5_24
LBB5_22:
	ldp	w0, w1, [x19]
	mov	x2, x20
	b	LBB5_52
LBB5_23:
	mov	x23, #0                         ; =0x0
LBB5_24:
Lloh32:
	adrp	x11, lJTI5_1@PAGE
Lloh33:
	add	x11, x11, lJTI5_1@PAGEOFF
	b	LBB5_29
LBB5_25:                                ;   in Loop: Header=BB5_29 Depth=1
	sub	x13, x12, #1
LBB5_26:                                ;   in Loop: Header=BB5_29 Depth=1
	add	w9, w9, #1
	str	w9, [x8, _lineno@PAGEOFF]
	add	x12, x13, #1
LBB5_27:                                ;   in Loop: Header=BB5_29 Depth=1
	str	x12, [x19, #32]
LBB5_28:                                ;   in Loop: Header=BB5_29 Depth=1
	mov	x21, x10
	mov	x10, x12
	subs	x22, x13, x21
	b.ne	LBB5_40
LBB5_29:                                ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB5_31 Depth 2
                                        ;     Child Loop BB5_34 Depth 2
                                        ;     Child Loop BB5_36 Depth 2
	add	x12, x10, #1
	b	LBB5_31
LBB5_30:                                ;   in Loop: Header=BB5_31 Depth=2
	str	x12, [x19, #32]
	add	x12, x12, #1
LBB5_31:                                ;   Parent Loop BB5_29 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldurb	w14, [x12, #-1]
	cmp	w14, #47
	b.hi	LBB5_30
; %bb.32:                               ;   in Loop: Header=BB5_31 Depth=2
	adr	x13, LBB5_25
	ldrb	w15, [x11, x14]
	add	x13, x13, x15, lsl #2
	br	x13
LBB5_33:                                ;   in Loop: Header=BB5_31 Depth=2
	ldrb	w13, [x12]
	cmp	w13, #47
	b.ne	LBB5_30
LBB5_34:                                ;   Parent Loop BB5_29 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	str	x12, [x19, #32]
	ldrb	w10, [x12], #1
	cmp	w10, #10
	b.ne	LBB5_34
; %bb.35:                               ;   in Loop: Header=BB5_29 Depth=1
	sub	x13, x12, #1
	mov	x10, x13
	b	LBB5_26
LBB5_36:                                ;   Parent Loop BB5_29 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	str	x12, [x19, #32]
	ldrb	w13, [x12], #1
	cmp	w13, #34
	ccmp	w13, #10, #4, ne
	b.ne	LBB5_36
; %bb.37:                               ;   in Loop: Header=BB5_29 Depth=1
	mov	x13, x12
	b	LBB5_27
LBB5_38:                                ;   in Loop: Header=BB5_29 Depth=1
	str	x12, [x19, #32]
	sub	x13, x12, #1
	cmp	w14, #44
	b.ne	LBB5_28
; %bb.39:                               ;   in Loop: Header=BB5_29 Depth=1
	add	x12, x12, #1
	b	LBB5_27
LBB5_40:
	ldrsb	w8, [x21]
	sub	w9, w8, #48
	cmp	w9, #9
	b.hi	LBB5_42
; %bb.41:
	mov	x0, x21
	mov	x1, #0                          ; =0x0
	mov	w2, #0                          ; =0x0
	bl	_strtol
	mov	x21, x0
	b	LBB5_50
LBB5_42:
	and	w8, w8, #0xff
	cmp	w8, #39
	b.ne	LBB5_45
; %bb.43:
	ldrsb	x21, [x21, #1]
	ldurb	w8, [x13, #-1]
	cmp	w8, #39
	b.eq	LBB5_50
; %bb.44:
Lloh34:
	adrp	x0, l_.str.7@PAGE
Lloh35:
	add	x0, x0, l_.str.7@PAGEOFF
	bl	_compile_err
	b	LBB5_50
LBB5_45:
Lloh36:
	adrp	x1, l_.str.8@PAGE
Lloh37:
	add	x1, x1, l_.str.8@PAGEOFF
	mov	x0, x21
	mov	x2, x22
	bl	_memcmp
	cbz	w0, LBB5_49
; %bb.46:
Lloh38:
	adrp	x1, l_.str.9@PAGE
Lloh39:
	add	x1, x1, l_.str.9@PAGEOFF
	mov	x0, x21
	mov	x2, x22
	bl	_memcmp
	cbz	w0, LBB5_48
; %bb.47:
Lloh40:
	adrp	x0, l_.str.10@PAGE
Lloh41:
	add	x0, x0, l_.str.10@PAGEOFF
	bl	_compile_err
LBB5_48:
	mov	x21, #0                         ; =0x0
	b	LBB5_50
LBB5_49:
	mov	w21, #1                         ; =0x1
LBB5_50:
	ldrb	w8, [x23]
	cmp	w8, #43
	b.ne	LBB5_53
; %bb.51:
	ldp	w0, w1, [x19]
	add	w2, w21, w20
LBB5_52:
	bl	_emit_mov
LBB5_53:
	mov	w0, #1                          ; =0x1
LBB5_54:
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp], #64             ; 16-byte Folded Reload
	ret
	.loh AdrpAdd	Lloh30, Lloh31
	.loh AdrpAdd	Lloh32, Lloh33
	.loh AdrpAdd	Lloh34, Lloh35
	.loh AdrpAdd	Lloh36, Lloh37
	.loh AdrpAdd	Lloh38, Lloh39
	.loh AdrpAdd	Lloh40, Lloh41
	.cfi_endproc
	.section	__TEXT,__const
lJTI5_0:
	.byte	(LBB5_15-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_17-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_15-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_13-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_15-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_7-LBB5_7)>>2
	.byte	(LBB5_10-LBB5_7)>>2
lJTI5_1:
	.byte	(LBB5_38-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_25-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_38-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_36-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_38-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_30-LBB5_25)>>2
	.byte	(LBB5_33-LBB5_25)>>2
                                        ; -- End function
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_expr_line                      ; -- Begin function expr_line
	.p2align	2
_expr_line:                             ; @expr_line
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #96
	stp	x26, x25, [sp, #16]             ; 16-byte Folded Spill
	stp	x24, x23, [sp, #32]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #48]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #64]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #80]             ; 16-byte Folded Spill
	add	x29, sp, #80
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	.cfi_offset w25, -72
	.cfi_offset w26, -80
	mov	x19, x2
	stp	x0, x1, [sp]
	mov	x0, sp
	mov	x1, x2
	bl	_expr
	mov	x20, x0
	cbz	w0, LBB6_29
; %bb.1:
Lloh42:
	adrp	x22, __DefaultRuneLocale@GOTPAGE
Lloh43:
	ldr	x22, [x22, __DefaultRuneLocale@GOTPAGEOFF]
	adrp	x23, _lineno@PAGE
	mov	w24, #1                         ; =0x1
	mov	x25, #1025                      ; =0x401
	movk	x25, #4097, lsl #32
Lloh44:
	adrp	x26, ___stdoutp@GOTPAGE
Lloh45:
	ldr	x26, [x26, ___stdoutp@GOTPAGEOFF]
Lloh46:
	adrp	x21, l_.str.23@PAGE
Lloh47:
	add	x21, x21, l_.str.23@PAGEOFF
	b	LBB6_4
LBB6_2:                                 ;   in Loop: Header=BB6_4 Depth=1
	ldr	x3, [x26]
	mov	w1, #1                          ; =0x1
	bl	_fwrite
LBB6_3:                                 ;   in Loop: Header=BB6_4 Depth=1
	ldr	x1, [x26]
	mov	w0, #10                         ; =0xa
	bl	_fputc
	mov	x0, sp
	mov	x1, x19
	bl	_expr
	tbz	w0, #0, LBB6_28
LBB6_4:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB6_9 Depth 2
                                        ;       Child Loop BB6_12 Depth 3
                                        ;         Child Loop BB6_17 Depth 4
                                        ;       Child Loop BB6_25 Depth 3
	ldr	x8, [sp, #8]
	ldrb	w9, [x8]
	cmp	w9, #44
	b.ne	LBB6_28
; %bb.5:                                ;   in Loop: Header=BB6_4 Depth=1
	ldrsb	w0, [x8, #1]
	tbnz	w0, #31, LBB6_7
; %bb.6:                                ;   in Loop: Header=BB6_4 Depth=1
	add	x8, x22, w0, uxtw #2
	ldr	w8, [x8, #60]
	and	w0, w8, #0x4000
	cbnz	w0, LBB6_8
	b	LBB6_28
LBB6_7:                                 ;   in Loop: Header=BB6_4 Depth=1
	mov	w1, #16384                      ; =0x4000
	bl	___maskrune
	cbz	w0, LBB6_28
LBB6_8:                                 ;   in Loop: Header=BB6_4 Depth=1
	ldr	w8, [x19, #4]
	add	w8, w8, #1
	str	w8, [x19, #4]
	ldr	w8, [x23, _lineno@PAGEOFF]
	ldr	x11, [x19, #32]
	ldr	x9, [x19, #48]
LBB6_9:                                 ;   Parent Loop BB6_4 Depth=1
                                        ; =>  This Loop Header: Depth=2
                                        ;       Child Loop BB6_12 Depth 3
                                        ;         Child Loop BB6_17 Depth 4
                                        ;       Child Loop BB6_25 Depth 3
	stp	x11, xzr, [sp]
	mov	x0, x11
	mov	x10, x11
	b	LBB6_12
LBB6_10:                                ;   in Loop: Header=BB6_12 Depth=3
	mov	w11, w12
	cmp	w12, #44
	lsl	x11, x24, x11
	and	x11, x11, x25
	ccmp	x11, #0, #4, ls
	b.ne	LBB6_20
LBB6_11:                                ;   in Loop: Header=BB6_12 Depth=3
	add	x10, x10, #1
	str	x10, [x19, #32]
LBB6_12:                                ;   Parent Loop BB6_4 Depth=1
                                        ;     Parent Loop BB6_9 Depth=2
                                        ; =>    This Loop Header: Depth=3
                                        ;         Child Loop BB6_17 Depth 4
	ldrb	w11, [x10]
	cmp	w11, #47
	b.eq	LBB6_15
; %bb.13:                               ;   in Loop: Header=BB6_12 Depth=3
	cmp	w11, #34
	b.eq	LBB6_24
; %bb.14:                               ;   in Loop: Header=BB6_12 Depth=3
	mov	x12, x11
	cmp	w11, #10
	b.ne	LBB6_10
	b	LBB6_19
LBB6_15:                                ;   in Loop: Header=BB6_12 Depth=3
	ldrb	w11, [x10, #1]
	cmp	w11, #47
	b.ne	LBB6_11
; %bb.16:                               ;   in Loop: Header=BB6_12 Depth=3
	add	x10, x10, #1
LBB6_17:                                ;   Parent Loop BB6_4 Depth=1
                                        ;     Parent Loop BB6_9 Depth=2
                                        ;       Parent Loop BB6_12 Depth=3
                                        ; =>      This Inner Loop Header: Depth=4
	str	x10, [x19, #32]
	ldrb	w11, [x10], #1
	cmp	w11, #10
	b.ne	LBB6_17
; %bb.18:                               ;   in Loop: Header=BB6_12 Depth=3
	sub	x0, x10, #1
	str	x0, [sp]
	ldurb	w11, [x10, #-1]
	mov	w12, #10                        ; =0xa
	mov	x10, x0
	cmp	w11, #10
	b.ne	LBB6_10
LBB6_19:                                ;   in Loop: Header=BB6_12 Depth=3
	add	w8, w8, #1
	str	w8, [x23, _lineno@PAGEOFF]
	b	LBB6_10
LBB6_20:                                ;   in Loop: Header=BB6_9 Depth=2
	add	x11, x10, #1
	str	x11, [x19, #32]
	str	x10, [sp, #8]
	cmp	w12, #44
	b.ne	LBB6_22
; %bb.21:                               ;   in Loop: Header=BB6_9 Depth=2
	add	x11, x10, #2
	str	x11, [x19, #32]
LBB6_22:                                ;   in Loop: Header=BB6_9 Depth=2
	cmp	x10, x9
	b.hi	LBB6_27
LBB6_23:                                ;   in Loop: Header=BB6_9 Depth=2
	subs	x2, x10, x0
	b.eq	LBB6_9
	b	LBB6_2
LBB6_24:                                ;   in Loop: Header=BB6_9 Depth=2
	add	x10, x10, #1
LBB6_25:                                ;   Parent Loop BB6_4 Depth=1
                                        ;     Parent Loop BB6_9 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	str	x10, [x19, #32]
	ldrb	w11, [x10], #1
	cmp	w11, #34
	ccmp	w11, #10, #4, ne
	b.ne	LBB6_25
; %bb.26:                               ;   in Loop: Header=BB6_9 Depth=2
	str	x10, [x19, #32]
	str	x10, [sp, #8]
	mov	x11, x10
	cmp	x10, x9
	b.ls	LBB6_23
LBB6_27:                                ;   in Loop: Header=BB6_4 Depth=1
	stp	xzr, xzr, [sp]
	ldr	x1, [x26]
	mov	x0, x21
	bl	_fputs
	b	LBB6_3
LBB6_28:
	str	wzr, [x19, #4]
LBB6_29:
	mov	x0, x20
	ldp	x29, x30, [sp, #80]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #64]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #48]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #32]             ; 16-byte Folded Reload
	ldp	x26, x25, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #96
	ret
	.loh AdrpAdd	Lloh46, Lloh47
	.loh AdrpLdrGot	Lloh44, Lloh45
	.loh AdrpLdrGot	Lloh42, Lloh43
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__literal8,8byte_literals
	.p2align	3, 0x0                          ; -- Begin function parse
lCPI7_0:
	.long	2                               ; 0x2
	.long	0                               ; 0x0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_parse
	.p2align	2
_parse:                                 ; @parse
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #96
	stp	x24, x23, [sp, #32]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #48]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #64]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #80]             ; 16-byte Folded Spill
	add	x29, sp, #80
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	mov	x19, x1
	mov	x23, x0
	ldp	x20, x21, [x0]
	mov	x0, x20
	mov	x1, x21
	mov	x2, x19
	bl	_expr_line
	tbnz	w0, #0, LBB7_42
; %bb.1:
	sub	x22, x21, x20
Lloh48:
	adrp	x1, l_.str.11@PAGE
Lloh49:
	add	x1, x1, l_.str.11@PAGEOFF
	mov	x0, x20
	mov	x2, x22
	bl	_memcmp
	cbz	w0, LBB7_24
; %bb.2:
	cmp	x22, #2
	b.lo	LBB7_4
; %bb.3:
	mov	x8, x21
	ldrh	w9, [x8, #-2]!
	mov	w10, #15933                     ; =0x3e3d
	cmp	w9, w10
	b.eq	LBB7_27
LBB7_4:
	ldrsb	w24, [x20]
	tbnz	w24, #31, LBB7_25
; %bb.5:
Lloh50:
	adrp	x8, __DefaultRuneLocale@GOTPAGE
Lloh51:
	ldr	x8, [x8, __DefaultRuneLocale@GOTPAGEOFF]
	add	x8, x8, w24, uxtw #2
	ldr	w8, [x8, #60]
	and	w9, w8, #0x1000
	cmp	w24, #95
	ccmp	w9, #0, #0, ne
	b.ne	LBB7_26
; %bb.6:
	and	w0, w8, #0x8000
	cbz	w0, LBB7_34
LBB7_7:
	adrp	x8, _lineno@PAGE
	ldr	w9, [x8, _lineno@PAGEOFF]
	ldr	x12, [x19, #32]
Lloh52:
	adrp	x10, lJTI7_0@PAGE
Lloh53:
	add	x10, x10, lJTI7_0@PAGEOFF
	ldr	x11, [x19, #48]
LBB7_8:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB7_10 Depth 2
                                        ;     Child Loop BB7_13 Depth 2
                                        ;     Child Loop BB7_15 Depth 2
	add	x13, x12, #1
	b	LBB7_10
LBB7_9:                                 ;   in Loop: Header=BB7_10 Depth=2
	str	x13, [x19, #32]
	add	x13, x13, #1
LBB7_10:                                ;   Parent Loop BB7_8 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldurb	w15, [x13, #-1]
	cmp	w15, #47
	b.hi	LBB7_9
; %bb.11:                               ;   in Loop: Header=BB7_10 Depth=2
	adr	x14, LBB7_9
	ldrb	w16, [x10, x15]
	add	x14, x14, x16, lsl #2
	br	x14
LBB7_12:                                ;   in Loop: Header=BB7_10 Depth=2
	ldrb	w14, [x13]
	cmp	w14, #47
	b.ne	LBB7_9
LBB7_13:                                ;   Parent Loop BB7_8 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	str	x13, [x19, #32]
	ldrb	w12, [x13], #1
	cmp	w12, #10
	b.ne	LBB7_13
; %bb.14:                               ;   in Loop: Header=BB7_8 Depth=1
	sub	x14, x13, #1
	mov	x12, x14
	b	LBB7_20
LBB7_15:                                ;   Parent Loop BB7_8 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	str	x13, [x19, #32]
	ldrb	w14, [x13], #1
	cmp	w14, #34
	ccmp	w14, #10, #4, ne
	b.ne	LBB7_15
; %bb.16:                               ;   in Loop: Header=BB7_8 Depth=1
	mov	x14, x13
	b	LBB7_21
LBB7_17:                                ;   in Loop: Header=BB7_8 Depth=1
	str	x13, [x19, #32]
	sub	x14, x13, #1
	cmp	w15, #44
	b.ne	LBB7_22
; %bb.18:                               ;   in Loop: Header=BB7_8 Depth=1
	add	x13, x13, #1
	b	LBB7_21
LBB7_19:                                ;   in Loop: Header=BB7_8 Depth=1
	sub	x14, x13, #1
LBB7_20:                                ;   in Loop: Header=BB7_8 Depth=1
	add	w9, w9, #1
	str	w9, [x8, _lineno@PAGEOFF]
	add	x13, x14, #1
LBB7_21:                                ;   in Loop: Header=BB7_8 Depth=1
	str	x13, [x19, #32]
LBB7_22:                                ;   in Loop: Header=BB7_8 Depth=1
	mov	x0, x12
	mov	x12, x13
	cmp	x14, x11
	b.hi	LBB7_30
; %bb.23:                               ;   in Loop: Header=BB7_8 Depth=1
	cmp	x14, x0
	b.eq	LBB7_8
	b	LBB7_31
LBB7_24:
	str	wzr, [x19]
	b	LBB7_42
LBB7_25:
	mov	x0, x24
	mov	w1, #4096                       ; =0x1000
	bl	___maskrune
	cbz	w0, LBB7_33
LBB7_26:
	mov	w8, #1                          ; =0x1
	str	w8, [x19]
	ldr	q0, [x23]
	str	q0, [x19, #16]
	b	LBB7_42
LBB7_27:
	stp	x20, x8, [sp, #16]
	mov	x11, x19
	ldp	x10, x9, [x11, #16]!
	cmp	x9, x10
	b.eq	LBB7_36
; %bb.28:
	cmp	x8, x20
	b.ne	LBB7_36
; %bb.29:
	stp	xzr, xzr, [x11]
	stp	x10, x9, [sp]
	mov	x0, sp
	bl	_emit_fn_call
	b	LBB7_41
LBB7_30:
	mov	x0, #0                          ; =0x0
	mov	x14, #0                         ; =0x0
LBB7_31:
	sub	x2, x14, x0
Lloh54:
	adrp	x1, l_.str.14@PAGE
Lloh55:
	add	x1, x1, l_.str.14@PAGEOFF
	bl	_memcmp
	cbnz	w0, LBB7_42
; %bb.32:
Lloh56:
	adrp	x0, l_.str.15@PAGE
Lloh57:
	add	x0, x0, l_.str.15@PAGEOFF
	bl	_printf
	mov	w8, #3                          ; =0x3
	ldr	w9, [x19, #8]
	add	w10, w9, #1
	stp	w9, w10, [x19, #4]
	str	w8, [x19]
	b	LBB7_42
LBB7_33:
	mov	x0, x24
	mov	w1, #32768                      ; =0x8000
	bl	___maskrune
	cbnz	w0, LBB7_7
LBB7_34:
Lloh58:
	adrp	x0, l_.str.16@PAGE
Lloh59:
	add	x0, x0, l_.str.16@PAGEOFF
	bl	_compile_err
Lloh60:
	adrp	x8, ___stderrp@GOTPAGE
Lloh61:
	ldr	x8, [x8, ___stderrp@GOTPAGEOFF]
Lloh62:
	ldr	x19, [x8]
	cmp	x20, x21
	b.eq	LBB7_38
; %bb.35:
	mov	x0, x20
	mov	w1, #1                          ; =0x1
	mov	x2, x22
	mov	x3, x19
	bl	_fwrite
	b	LBB7_39
LBB7_36:
	cmp	x8, x20
	b.eq	LBB7_40
; %bb.37:
	add	x0, sp, #16
	bl	_emit_fn_call
	b	LBB7_41
LBB7_38:
Lloh63:
	adrp	x0, l_.str.23@PAGE
Lloh64:
	add	x0, x0, l_.str.23@PAGEOFF
	mov	x1, x19
	bl	_fputs
LBB7_39:
	mov	w0, #10                         ; =0xa
	mov	x1, x19
	ldp	x29, x30, [sp, #80]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #64]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #48]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #96
	b	_fputc
LBB7_40:
Lloh65:
	adrp	x0, l_.str.13@PAGE
Lloh66:
	add	x0, x0, l_.str.13@PAGEOFF
	bl	_compile_err
LBB7_41:
Lloh67:
	adrp	x8, lCPI7_0@PAGE
Lloh68:
	ldr	d0, [x8, lCPI7_0@PAGEOFF]
	str	d0, [x19]
	strb	wzr, [x19, #56]
LBB7_42:
	ldp	x29, x30, [sp, #80]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #64]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #48]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #96
	ret
	.loh AdrpAdd	Lloh48, Lloh49
	.loh AdrpLdrGot	Lloh50, Lloh51
	.loh AdrpAdd	Lloh52, Lloh53
	.loh AdrpAdd	Lloh54, Lloh55
	.loh AdrpAdd	Lloh56, Lloh57
	.loh AdrpLdrGotLdr	Lloh60, Lloh61, Lloh62
	.loh AdrpAdd	Lloh58, Lloh59
	.loh AdrpAdd	Lloh63, Lloh64
	.loh AdrpAdd	Lloh65, Lloh66
	.loh AdrpLdr	Lloh67, Lloh68
	.cfi_endproc
	.section	__TEXT,__const
lJTI7_0:
	.byte	(LBB7_17-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_19-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_17-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_15-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_17-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_9-LBB7_9)>>2
	.byte	(LBB7_12-LBB7_9)>>2
                                        ; -- End function
	.section	__TEXT,__literal8,8byte_literals
	.p2align	3, 0x0                          ; -- Begin function main
lCPI8_0:
	.long	2                               ; 0x2
	.long	0                               ; 0x0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #176
	stp	x26, x25, [sp, #96]             ; 16-byte Folded Spill
	stp	x24, x23, [sp, #112]            ; 16-byte Folded Spill
	stp	x22, x21, [sp, #128]            ; 16-byte Folded Spill
	stp	x20, x19, [sp, #144]            ; 16-byte Folded Spill
	stp	x29, x30, [sp, #160]            ; 16-byte Folded Spill
	add	x29, sp, #160
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	.cfi_offset w25, -72
	.cfi_offset w26, -80
	mov	x20, x1
	ldr	x19, [x1, #8]
Lloh69:
	adrp	x1, l_.str.17@PAGE
Lloh70:
	add	x1, x1, l_.str.17@PAGEOFF
	mov	x0, x19
	bl	_fopen
	cbz	x0, LBB8_27
; %bb.1:
	mov	x22, x0
	mov	x1, #0                          ; =0x0
	mov	w2, #2                          ; =0x2
	bl	_fseek
	mov	x0, x22
	bl	_ftell
	mov	x21, x0
	mov	x0, x22
	bl	_rewind
	mov	x0, x21
	bl	_malloc
	mov	x20, x0
	mov	x1, x21
	bl	_bzero
	cbz	x20, LBB8_28
; %bb.2:
	mov	w23, #1                         ; =0x1
	mov	x0, x20
	mov	w1, #1                          ; =0x1
	mov	x2, x21
	mov	x3, x22
	bl	_fread
	cmp	x0, x21
	b.hi	LBB8_29
; %bb.3:
	add	x22, x20, x21
	bl	_emit_init
	bl	_emit_mainfn
Lloh71:
	adrp	x8, lCPI8_0@PAGE
Lloh72:
	ldr	d0, [x8, lCPI8_0@PAGEOFF]
	str	d0, [sp, #32]
	str	wzr, [sp, #40]
	stp	xzr, xzr, [sp, #48]
	stp	x20, x20, [sp, #64]
	str	x22, [sp, #80]
	strb	w23, [sp, #88]
	cmp	x21, #1
	b.lt	LBB8_25
; %bb.4:
	adrp	x21, _lineno@PAGE
	ldr	w8, [x21, _lineno@PAGEOFF]
	mov	w23, #1                         ; =0x1
	mov	x24, #1025                      ; =0x401
	movk	x24, #4097, lsl #32
Lloh73:
	adrp	x25, ___stdoutp@GOTPAGE
Lloh74:
	ldr	x25, [x25, ___stdoutp@GOTPAGEOFF]
	b	LBB8_6
LBB8_5:                                 ;   in Loop: Header=BB8_6 Depth=1
	stp	xzr, xzr, [sp, #16]
	cmp	x20, x22
	b.hs	LBB8_25
LBB8_6:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB8_9 Depth 2
                                        ;       Child Loop BB8_14 Depth 3
                                        ;     Child Loop BB8_23 Depth 2
	stp	x20, xzr, [sp, #16]
	mov	x0, x20
	mov	x9, x20
	b	LBB8_9
LBB8_7:                                 ;   in Loop: Header=BB8_9 Depth=2
	mov	w11, w10
	cmp	w10, #44
	lsl	x11, x23, x11
	and	x11, x11, x24
	ccmp	x11, #0, #4, ls
	b.ne	LBB8_17
LBB8_8:                                 ;   in Loop: Header=BB8_9 Depth=2
	add	x9, x9, #1
	str	x9, [sp, #64]
LBB8_9:                                 ;   Parent Loop BB8_6 Depth=1
                                        ; =>  This Loop Header: Depth=2
                                        ;       Child Loop BB8_14 Depth 3
	ldrb	w11, [x9]
	cmp	w11, #47
	b.eq	LBB8_12
; %bb.10:                               ;   in Loop: Header=BB8_9 Depth=2
	cmp	w11, #34
	b.eq	LBB8_22
; %bb.11:                               ;   in Loop: Header=BB8_9 Depth=2
	mov	x10, x11
	cmp	w11, #10
	b.ne	LBB8_7
	b	LBB8_16
LBB8_12:                                ;   in Loop: Header=BB8_9 Depth=2
	ldrb	w10, [x9, #1]
	cmp	w10, #47
	b.ne	LBB8_8
; %bb.13:                               ;   in Loop: Header=BB8_9 Depth=2
	add	x9, x9, #1
LBB8_14:                                ;   Parent Loop BB8_6 Depth=1
                                        ;     Parent Loop BB8_9 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	str	x9, [sp, #64]
	ldrb	w10, [x9], #1
	cmp	w10, #10
	b.ne	LBB8_14
; %bb.15:                               ;   in Loop: Header=BB8_9 Depth=2
	sub	x0, x9, #1
	str	x0, [sp, #16]
	ldurb	w11, [x9, #-1]
	mov	x9, x0
	cmp	w11, #10
	b.ne	LBB8_7
LBB8_16:                                ;   in Loop: Header=BB8_9 Depth=2
	add	w8, w8, #1
	str	w8, [x21, _lineno@PAGEOFF]
	b	LBB8_7
LBB8_17:                                ;   in Loop: Header=BB8_6 Depth=1
	add	x20, x9, #1
	str	x20, [sp, #64]
	str	x9, [sp, #24]
	cmp	w10, #44
	b.ne	LBB8_19
; %bb.18:                               ;   in Loop: Header=BB8_6 Depth=1
	add	x20, x9, #2
	str	x20, [sp, #64]
LBB8_19:                                ;   in Loop: Header=BB8_6 Depth=1
	cmp	x9, x22
	b.hi	LBB8_5
LBB8_20:                                ;   in Loop: Header=BB8_6 Depth=1
	subs	x2, x9, x0
	b.eq	LBB8_6
; %bb.21:                               ;   in Loop: Header=BB8_6 Depth=1
	ldr	x3, [x25]
	mov	w1, #1                          ; =0x1
	bl	_fwrite
	ldr	x1, [x25]
	mov	w0, #10                         ; =0xa
	bl	_fputc
	add	x0, sp, #16
	add	x1, sp, #32
	bl	_parse
	ldr	w8, [x21, _lineno@PAGEOFF]
	ldr	x20, [sp, #64]
	ldr	x22, [sp, #80]
	cmp	x20, x22
	b.lo	LBB8_6
	b	LBB8_25
LBB8_22:                                ;   in Loop: Header=BB8_6 Depth=1
	add	x9, x9, #1
LBB8_23:                                ;   Parent Loop BB8_6 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	str	x9, [sp, #64]
	ldrb	w10, [x9], #1
	cmp	w10, #34
	ccmp	w10, #10, #4, ne
	b.ne	LBB8_23
; %bb.24:                               ;   in Loop: Header=BB8_6 Depth=1
	str	x9, [sp, #64]
	str	x9, [sp, #24]
	mov	x20, x9
	cmp	x9, x22
	b.ls	LBB8_20
	b	LBB8_5
LBB8_25:
	add	x0, sp, #32
	bl	_emit_fn_prologue
	add	x0, sp, #32
	bl	_emit_fn_epilogue
	bl	_emit_ret
	mov	x0, x19
	bl	_strlen
	mov	x20, x0
	add	x21, x0, #1
	mov	x0, x21
	bl	_malloc
	mov	x22, x0
	mov	x1, x21
	bl	_bzero
	sub	x2, x20, #1
	mov	x0, x22
	mov	x1, x19
	bl	_strncpy
	add	x8, x20, x22
	mov	w9, #115                        ; =0x73
	sturb	w9, [x8, #-2]
Lloh75:
	adrp	x1, l_.str.21@PAGE
Lloh76:
	add	x1, x1, l_.str.21@PAGEOFF
	bl	_fopen
	cbz	x0, LBB8_31
; %bb.26:
	bl	_emit
	adrp	x8, _has_compile_err@PAGE
	ldrb	w0, [x8, _has_compile_err@PAGEOFF]
	ldp	x29, x30, [sp, #160]            ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #144]            ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #128]            ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #112]            ; 16-byte Folded Reload
	ldp	x26, x25, [sp, #96]             ; 16-byte Folded Reload
	add	sp, sp, #176
	ret
LBB8_27:
Lloh77:
	adrp	x8, ___stderrp@GOTPAGE
Lloh78:
	ldr	x8, [x8, ___stderrp@GOTPAGEOFF]
Lloh79:
	ldr	x0, [x8]
	ldr	x8, [x20, #8]
	str	x8, [sp]
Lloh80:
	adrp	x1, l_.str.18@PAGE
Lloh81:
	add	x1, x1, l_.str.18@PAGEOFF
	b	LBB8_30
LBB8_28:
Lloh82:
	adrp	x8, ___stderrp@GOTPAGE
Lloh83:
	ldr	x8, [x8, ___stderrp@GOTPAGEOFF]
Lloh84:
	ldr	x1, [x8]
Lloh85:
	adrp	x0, l_.str.19@PAGE
Lloh86:
	add	x0, x0, l_.str.19@PAGEOFF
	bl	_fputs
	mov	w0, #1                          ; =0x1
	bl	_exit
LBB8_29:
Lloh87:
	adrp	x8, ___stderrp@GOTPAGE
Lloh88:
	ldr	x8, [x8, ___stderrp@GOTPAGEOFF]
Lloh89:
	ldr	x8, [x8]
	stp	x21, x0, [sp]
Lloh90:
	adrp	x1, l_.str.20@PAGE
Lloh91:
	add	x1, x1, l_.str.20@PAGEOFF
	mov	x0, x8
LBB8_30:
	bl	_fprintf
	mov	w0, #1                          ; =0x1
	bl	_exit
LBB8_31:
Lloh92:
	adrp	x8, ___stderrp@GOTPAGE
Lloh93:
	ldr	x8, [x8, ___stderrp@GOTPAGEOFF]
Lloh94:
	ldr	x3, [x8]
Lloh95:
	adrp	x0, l_.str.22@PAGE
Lloh96:
	add	x0, x0, l_.str.22@PAGEOFF
	mov	w1, #29                         ; =0x1d
	mov	w2, #1                          ; =0x1
	bl	_fwrite
	mov	w0, #1                          ; =0x1
	bl	_exit
	.loh AdrpAdd	Lloh69, Lloh70
	.loh AdrpLdr	Lloh71, Lloh72
	.loh AdrpLdrGot	Lloh73, Lloh74
	.loh AdrpAdd	Lloh75, Lloh76
	.loh AdrpAdd	Lloh80, Lloh81
	.loh AdrpLdrGotLdr	Lloh77, Lloh78, Lloh79
	.loh AdrpAdd	Lloh85, Lloh86
	.loh AdrpLdrGotLdr	Lloh82, Lloh83, Lloh84
	.loh AdrpAdd	Lloh90, Lloh91
	.loh AdrpLdrGotLdr	Lloh87, Lloh88, Lloh89
	.loh AdrpAdd	Lloh95, Lloh96
	.loh AdrpLdrGotLdr	Lloh92, Lloh93, Lloh94
	.cfi_endproc
                                        ; -- End function
	.section	__DATA,__data
	.globl	_lineno                         ; @lineno
	.p2align	2, 0x0
_lineno:
	.long	1                               ; 0x1

	.globl	_has_compile_err                ; @has_compile_err
.zerofill __DATA,__common,_has_compile_err,1,0
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
