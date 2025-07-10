	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 14, 0	sdk_version 15, 2
	.globl	_filelen                        ; -- Begin function filelen
	.p2align	2
_filelen:                               ; @filelen
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
	mov	x19, x0
	mov	x1, #0                          ; =0x0
	mov	w2, #2                          ; =0x2
	bl	_fseek
	mov	x0, x19
	bl	_ftell
	mov	x20, x0
	mov	x0, x19
	mov	x1, #0                          ; =0x0
	mov	w2, #0                          ; =0x0
	bl	_fseek
	mov	x0, x20
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp], #32             ; 16-byte Folded Reload
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_usage                          ; -- Begin function usage
	.p2align	2
_usage:                                 ; @usage
	.cfi_startproc
; %bb.0:
Lloh0:
	adrp	x0, l_str@PAGE
Lloh1:
	add	x0, x0, l_str@PAGEOFF
	b	_puts
	.loh AdrpAdd	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.globl	_read_file                      ; -- Begin function read_file
	.p2align	2
_read_file:                             ; @read_file
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
	mov	x19, x2
	mov	x20, x1
	mov	x21, x0
Lloh2:
	adrp	x0, l_.str@PAGE
Lloh3:
	add	x0, x0, l_.str@PAGEOFF
Lloh4:
	adrp	x1, l_.str.2@PAGE
Lloh5:
	add	x1, x1, l_.str.2@PAGEOFF
	bl	_fopen
	str	x0, [x21]
	mov	w0, #1024                       ; =0x400
	bl	_malloc
	str	x0, [x20]
	ldr	x22, [x21]
	mov	x0, x22
	mov	x1, #0                          ; =0x0
	mov	w2, #2                          ; =0x2
	bl	_fseek
	mov	x0, x22
	bl	_ftell
	mov	x23, x0
	mov	x0, x22
	mov	x1, #0                          ; =0x0
	mov	w2, #0                          ; =0x0
	bl	_fseek
	str	w23, [x19]
	ldr	x0, [x20]
	sxtw	x2, w23
	ldr	x3, [x21]
	mov	w1, #1                          ; =0x1
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp], #64             ; 16-byte Folded Reload
	b	_fread
	.loh AdrpAdd	Lloh4, Lloh5
	.loh AdrpAdd	Lloh2, Lloh3
	.cfi_endproc
                                        ; -- End function
	.globl	_read_file2                     ; -- Begin function read_file2
	.p2align	2
_read_file2:                            ; @read_file2
	.cfi_startproc
; %bb.0:
	stp	x22, x21, [sp, #-48]!           ; 16-byte Folded Spill
	stp	x20, x19, [sp, #16]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
Lloh6:
	adrp	x0, l_.str@PAGE
Lloh7:
	add	x0, x0, l_.str@PAGEOFF
Lloh8:
	adrp	x1, l_.str.2@PAGE
Lloh9:
	add	x1, x1, l_.str.2@PAGEOFF
	bl	_fopen
	mov	x19, x0
	mov	w0, #1024                       ; =0x400
	bl	_malloc
	mov	x20, x0
	mov	x0, x19
	mov	x1, #0                          ; =0x0
	mov	w2, #2                          ; =0x2
	bl	_fseek
	mov	x0, x19
	bl	_ftell
	mov	x21, x0
	mov	x0, x19
	mov	x1, #0                          ; =0x0
	mov	w2, #0                          ; =0x0
	bl	_fseek
	sxtw	x2, w21
	mov	x0, x20
	mov	w1, #1                          ; =0x1
	mov	x3, x19
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp], #48             ; 16-byte Folded Reload
	b	_fread
	.loh AdrpAdd	Lloh8, Lloh9
	.loh AdrpAdd	Lloh6, Lloh7
	.cfi_endproc
                                        ; -- End function
	.globl	_find_line_end                  ; -- Begin function find_line_end
	.p2align	2
_find_line_end:                         ; @find_line_end
	.cfi_startproc
; %bb.0:
	add	x0, x0, #1
LBB4_1:                                 ; =>This Inner Loop Header: Depth=1
	ldrb	w8, [x0], #1
	cmp	w8, #10
	ccmp	w8, #0, #4, ne
	b.ne	LBB4_1
; %bb.2:
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
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
	cmp	w0, #1
	b.ne	LBB5_2
; %bb.1:
	add	x0, sp, #24
	add	x1, sp, #16
	add	x2, sp, #12
	bl	_read_file
	ldr	x8, [sp, #16]
	str	x8, [sp]
Lloh10:
	adrp	x0, l_.str.3@PAGE
Lloh11:
	add	x0, x0, l_.str.3@PAGEOFF
	bl	_printf
	mov	w0, #0                          ; =0x0
	bl	_exit
LBB5_2:
	mov	x21, x1
	ldr	x8, [x1, #8]
	ldrb	w9, [x8]
	cmp	w9, #45
	b.ne	LBB5_6
; %bb.3:
	ldrb	w8, [x8, #1]
	cmp	w8, #100
	b.eq	LBB5_7
; %bb.4:
	cmp	w8, #99
	b.ne	LBB5_15
; %bb.5:
Lloh12:
	adrp	x0, l_.str@PAGE
Lloh13:
	add	x0, x0, l_.str@PAGEOFF
Lloh14:
	adrp	x1, l_.str.4@PAGE
Lloh15:
	add	x1, x1, l_.str.4@PAGEOFF
	bl	_fopen
	mov	x19, x0
	ldr	x20, [x21, #16]
	mov	x0, x20
	bl	_strlen
	sxtw	x2, w0
	mov	x0, x20
	mov	w1, #1                          ; =0x1
	mov	x3, x19
	bl	_fwrite
Lloh16:
	adrp	x0, l_.str.5@PAGE
Lloh17:
	add	x0, x0, l_.str.5@PAGEOFF
	mov	w1, #1                          ; =0x1
	mov	w2, #1                          ; =0x1
	mov	x3, x19
	bl	_fwrite
	mov	x0, x19
	bl	_fclose
	mov	w0, #0                          ; =0x0
	bl	_exit
LBB5_6:
	bl	_usage
	mov	w0, #0                          ; =0x0
	bl	_exit
LBB5_7:
Lloh18:
	adrp	x20, l_.str@PAGE
Lloh19:
	add	x20, x20, l_.str@PAGEOFF
Lloh20:
	adrp	x1, l_.str.2@PAGE
Lloh21:
	add	x1, x1, l_.str.2@PAGEOFF
	mov	x0, x20
	bl	_fopen
	mov	x23, x0
	mov	w0, #1024                       ; =0x400
	bl	_malloc
	mov	x19, x0
	mov	x0, x23
	mov	x1, #0                          ; =0x0
	mov	w2, #2                          ; =0x2
	bl	_fseek
	mov	x0, x23
	bl	_ftell
	mov	x22, x0
	mov	x0, x23
	mov	x1, #0                          ; =0x0
	mov	w2, #0                          ; =0x0
	bl	_fseek
	sxtw	x22, w22
	mov	x0, x19
	mov	w1, #1                          ; =0x1
	mov	x2, x22
	mov	x3, x23
	bl	_fread
Lloh22:
	adrp	x1, l_.str.6@PAGE
Lloh23:
	add	x1, x1, l_.str.6@PAGEOFF
	mov	x0, x20
	bl	_fopen
	mov	x20, x0
	ldr	x0, [x21, #16]
	bl	_atoi
	mov	x21, x19
	cmp	w0, #1
	b.lt	LBB5_12
; %bb.8:
	mov	w8, #0                          ; =0x0
	mov	x21, x19
LBB5_9:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB5_10 Depth 2
	add	x21, x21, #1
LBB5_10:                                ;   Parent Loop BB5_9 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w9, [x21], #1
	cmp	w9, #10
	ccmp	w9, #0, #4, ne
	b.ne	LBB5_10
; %bb.11:                               ;   in Loop: Header=BB5_9 Depth=1
	add	w8, w8, #1
	cmp	w8, w0
	b.ne	LBB5_9
LBB5_12:
	sub	x2, x21, x19
	mov	x0, x19
	mov	w1, #1                          ; =0x1
	mov	x3, x20
	bl	_fwrite
	add	x0, x21, #1
	mvn	x8, x21
	add	x9, x19, x22
	add	x2, x8, x9
LBB5_13:                                ; =>This Inner Loop Header: Depth=1
	ldrb	w8, [x0], #1
	sub	x2, x2, #1
	cmp	w8, #10
	ccmp	w8, #0, #4, ne
	b.ne	LBB5_13
; %bb.14:
	mov	w1, #1                          ; =0x1
	mov	x3, x20
	bl	_fwrite
	mov	x0, x20
	bl	_fclose
	mov	w0, #0                          ; =0x0
	bl	_exit
LBB5_15:
	bl	_usage
	mov	w0, #1                          ; =0x1
	bl	_exit
	.loh AdrpAdd	Lloh10, Lloh11
	.loh AdrpAdd	Lloh16, Lloh17
	.loh AdrpAdd	Lloh14, Lloh15
	.loh AdrpAdd	Lloh12, Lloh13
	.loh AdrpAdd	Lloh22, Lloh23
	.loh AdrpAdd	Lloh20, Lloh21
	.loh AdrpAdd	Lloh18, Lloh19
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"todo.txt"

	.section	__DATA,__const
	.globl	_FILE_NAME                      ; @FILE_NAME
	.p2align	3, 0x0
_FILE_NAME:
	.quad	l_.str

	.section	__TEXT,__cstring,cstring_literals
l_.str.2:                               ; @.str.2
	.asciz	"r"

l_.str.3:                               ; @.str.3
	.asciz	"%s"

l_.str.4:                               ; @.str.4
	.asciz	"a"

l_.str.5:                               ; @.str.5
	.asciz	"\n"

l_.str.6:                               ; @.str.6
	.asciz	"w"

l_str:                                  ; @str
	.asciz	"usage\ncreate entry test: -c test\ndelete entry at 0: -d 0"

.subsections_via_symbols
