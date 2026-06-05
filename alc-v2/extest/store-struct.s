void store_struct(x0 reg_t dst, x1 i64 offset, x2 const dtype_t *dtype, x3 const dyn_agg_member *args) {

_store_struct:
        ;prolog
	sub	sp, sp, #0x1a0
	stp	x28, x27, [sp, #0x140] ; use all 10 calee's
	stp	x26, x25, [sp, #0x150]
	stp	x24, x23, [sp, #0x160]
	stp	x22, x21, [sp, #0x170]
	stp	x20, x19, [sp, #0x180]
	stp	x29, x30, [sp, #0x190]
	add	x29, sp, #0x190

	mov	x23, x3 ; args
	mov	x20, x2 ; dtype
	str	x1, [sp, #0x20] ; offset
	mov	x22, x0 ; dst
	add	x26, sp, #0xb0 ; ptr to some stack obj
	ldr	x27, [x2] ; pbase
	ldp	x9, x8, [x3] ; arg. begin, cur
	sub	x8, x8, x9 ; sizeof args
	ldr	x9, [x2, #0x28] ; decl_len of dtype
	cbz	x9, lbl0 ; when len is zero
	add	x9, x20, x9, lsl #2 ; getting declarator of index x9(decl_len of dtype)
	ldr	w9, [x9, #0x4] ; get top of the decl
	and	w9, w9, #0x3 ; get lower two bits as tag field is a bit-field
	cmp	w9, #0x1 ; if dk_array
	cset	w25, eq ; is_arr
	b	lbl1
lbl0:
	mov	w25, #0x0 ; zero out if decl_len was zero
lbl1:
	asr	x10, x8, #6 ; divide by 64 to get count of arg cnt
	stur	wzr, [x29, #-0x64] ; zero out some stack obj
	stur	xzr, [x29, #-0x70]
	ldr	x8, [x27] ; size of base type
	mov	w9, #0x8
	cmp	x8, #0x8 ; cmp size with 8
	csel	x8, x8, x9, lo ;max 8
	stp	x8, x10, [sp, #0x28] store arg cnt and rsize to the stack
	add	x21, x26, #0x10
	add	x8, sp, #0x70
	add	x9, x8, #0x10
	add	x8, sp, #0x38
	add	x8, x8, #0x10
	stp	x8, x9, [sp, #0x10]
	mov	w28, #0x3
	b	0x100005368
	ldur	x8, [x29, #-0xa0]
	ldr	x8, [x8]
	ldur	x9, [x29, #-0x70]
	add	x8, x9, x8
	stur	x8, [x29, #-0x70]
	ldur	w8, [x29, #-0x64]
	add	w8, w8, #0x1
	stur	w8, [x29, #-0x64]
	ldursw	x19, [x29, #-0x64]
	ldr	x8, [sp, #0x30]
	cmp	x8, x19
	b.le	0x100005740
	mov	x8, x20
	tbnz	w25, #0x0, 0x10000538c
	ldr	x8, [x27, #0x20]
	mov	w9, #0x48
	smaddl	x8, w19, w9, x8
	ldp	q1, q0, [x8, #0x10]
	ldr	q2, [x8]
	stp	q1, q0, [x26, #0x50]
	str	q2, [x26, #0x40]
	ldur	x8, [x29, #-0xa0]
	ldrb	w8, [x8, #0x9]
	cmp	w8, #0x2
	b.ne	0x1000053f4
	ldr	x8, [x23]
	add	x8, x8, x19, lsl #6
	ldrb	w9, [x8, #0x38]
	cmp	w9, #0x1
	b.eq	0x100005510
	cmp	w9, #0x3
	b.ne	0x100005590
	ldr	x9, [x20, #0x28]
	cbz	x9, 0x10000554c
	ldp	q1, q0, [x20, #0x10]
	stp	q1, q0, [sp, #0x80]
	ldr	q0, [x20]
	str	q0, [sp, #0x70]
	ldr	x9, [sp, #0x98]
	cbz	x9, 0x10000555c
	sub	x9, x9, #0x1
	str	x9, [sp, #0x98]
	b	0x10000555c
	str	wzr, [sp, #0xb0]
	ldr	x8, [sp, #0x28]
	strb	w8, [sp, #0xb4]
	strb	w28, [sp, #0xb5]
	strh	wzr, [sp, #0xb6]
	str	x27, [sp, #0xb8]
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [x21]
	str	xzr, [x21, #0x20]
	add	x0, sp, #0xb0
	sub	x3, x29, #0x64
	sub	x4, x29, #0x70
	mov	x1, x20
	mov	x24, x23
	mov	x2, x23
	bl	_emit_eightbyte_struct
	ldursw	x9, [x29, #-0x64]
	cmp	w19, w9
	b.eq	0x10000563c
	mov	x23, x0
	ldr	x10, [x27]
	ldur	x8, [x29, #-0x70]
	sub	x10, x10, x8
	cmp	x10, #0x8
	b.lo	0x100005478
	ldr	x10, [sp, #0x30]
	cmp	x10, x9
	b.le	0x100005478
	ldr	x10, [x24]
	add	x9, x10, x9, lsl #6
	ldrb	w9, [x9, #0x38]
	cmp	w9, #0x3
	b.ne	0x1000055f8
	mov	w6, #0x0
	mov	w5, #0x0
	ldr	x9, [sp, #0x28]
	ldr	x10, [sp, #0x20]
	add	x1, x8, x10
	ldp	q0, q1, [x22]
	str	q0, [sp, #0xb0]
	ldr	q0, [x22, #0x20]
	stp	q1, q0, [x26, #0x10]
	ldr	x8, [x22, #0x30]
	str	x8, [sp, #0xe0]
	str	wzr, [sp, #0x70]
	strb	w9, [sp, #0x74]
	mov	w10, #0x3
	strb	w10, [sp, #0x75]
	strh	wzr, [sp, #0x76]
	str	x27, [sp, #0x78]
	ldr	x8, [sp, #0x18]
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [x8]
	str	xzr, [x8, #0x20]
	mov	w8, #0x1
	str	w8, [sp, #0x38]
	strb	w9, [sp, #0x3c]
	mov	w28, #0x3
	strb	w10, [sp, #0x3d]
	strh	wzr, [sp, #0x3e]
	str	x27, [sp, #0x40]
	ldr	x8, [sp, #0x10]
	stp	q0, q0, [x8]
	str	xzr, [x8, #0x20]
	add	x0, sp, #0xb0
	add	x2, sp, #0x70
	add	x4, sp, #0x38
	mov	x3, x23
	bl	_emit_store_eightbytes
	mov	x23, x24
	b	0x100005368
	ldr	x8, [x8]
	cbnz	x8, 0x100005590
	ldur	x8, [x29, #-0x70]
	ldr	x9, [sp, #0x20]
	add	x1, x8, x9
	ldp	q0, q1, [x22]
	str	q0, [sp, #0xb0]
	ldr	q0, [x22, #0x20]
	stp	q1, q0, [x26, #0x10]
	ldr	x8, [x22, #0x30]
	str	x8, [sp, #0xe0]
	add	x0, sp, #0xb0
	sub	x2, x29, #0xa0
	bl	_emit_zerofill
	b	0x100005590
	ldp	q0, q1, [x26, #0x40]
	stp	q0, q1, [sp, #0x70]
	ldr	q0, [x26, #0x60]
	str	q0, [sp, #0x90]
	ldur	x9, [x29, #-0x70]
	ldr	x10, [sp, #0x20]
	add	x1, x9, x10
	ldr	x3, [x8]
	ldp	q0, q1, [x22]
	str	q0, [sp, #0xb0]
	ldr	q0, [x22, #0x20]
	stp	q1, q0, [x26, #0x10]
	ldr	x8, [x22, #0x30]
	str	x8, [sp, #0xe0]
	add	x0, sp, #0xb0
	add	x2, sp, #0x70
	bl	_store_struct
	ldur	x8, [x29, #-0x78]
	cbz	x8, 0x100005348
	sub	x9, x29, #0xa0
	add	x8, x9, x8, lsl #2
	ldr	w8, [x8, #0x4]
	ands	w9, w8, #0x3
	b.eq	0x1000055f0
	cmp	w9, #0x1
	b.ne	0x100005760
	asr	w19, w8, #2
	cmp	w8, #0x3
	b.gt	0x1000055dc
	adrp	x8, 11 ; 0x100010000
	ldr	x8, [x8, #0x28] ; literal pool symbol address: ___stderrp
	ldr	x0, [x8]
	str	x19, [sp]
	adrp	x1, 10 ; 0x10000f000
	add	x1, x1, #0x7ab ; literal pool for: "array length was <= 0 (%d)"
	bl	0x10000e9e8 ; symbol stub for: _fprintf
	ldur	x8, [x29, #-0xa0]
	ldr	x8, [x8]
	sxtw	x9, w19
	mul	x8, x8, x9
	b	0x100005350
	mov	w8, #0x8
	b	0x100005350
	mov	x2, x24
	mov	x8, #0x1
	movk	x8, #0x308, lsl #32
	stp	x8, x27, [sp, #0xb0]
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [x21]
	str	xzr, [x21, #0x20]
	add	x0, sp, #0xb0
	sub	x3, x29, #0x64
	sub	x4, x29, #0x70
	mov	x1, x20
	bl	_emit_eightbyte_struct
	mov	x5, x0
	mov	w9, #0x8
	mov	w6, #0x1
	ldur	x8, [x29, #-0x70]
	b	0x100005484
	ldur	x8, [x29, #-0x78]
	cbz	x8, 0x10000569c
	sub	x9, x29, #0xa0
	add	x8, x9, x8, lsl #2
	ldr	w8, [x8, #0x4]
	ands	w9, w8, #0x3
	b.eq	0x1000056a8
	cmp	w9, #0x1
	b.ne	0x100005764
	asr	w19, w8, #2
	cmp	w8, #0x3
	b.gt	0x100005688
	adrp	x8, 11 ; 0x100010000
	ldr	x8, [x8, #0x28] ; literal pool symbol address: ___stderrp
	ldr	x0, [x8]
	str	x19, [sp]
	adrp	x1, 10 ; 0x10000f000
	add	x1, x1, #0x7ab ; literal pool for: "array length was <= 0 (%d)"
	bl	0x10000e9e8 ; symbol stub for: _fprintf
	ldur	x8, [x29, #-0xa0]
	ldr	x8, [x8]
	sxtw	x9, w19
	mul	x8, x8, x9
	b	0x1000056ac
	ldur	x8, [x29, #-0xa0]
	ldr	x8, [x8]
	b	0x1000056ac
	mov	w8, #0x8
	str	x8, [sp]
	adrp	x1, 10 ; 0x10000f000
	add	x1, x1, #0xf5 ; literal pool for: "member size expected less than 16, but was %zd. member name: "
	mov	x0, #0x0
	bl	_compile_err
	ldur	x8, [x29, #-0xa0]
	ldp	x20, x21, [x8, #0x10]
	adrp	x19, 11 ; 0x100010000
	ldr	x19, [x19, #0x28] ; literal pool symbol address: ___stderrp
	ldr	x1, [x19]
	adrp	x0, 9 ; 0x10000e000
	add	x0, x0, #0xb34 ; literal pool for: "\033[31m"
	bl	0x10000ea00 ; symbol stub for: _fputs
	ldr	x19, [x19]
	cmp	x20, x21
	b.eq	0x10000570c
	sub	x2, x21, x20
	cmp	x2, #0x1
	b.lt	0x10000571c
	mov	x0, x20
	mov	w1, #0x1
	mov	x3, x19
	bl	0x10000ea3c ; symbol stub for: _fwrite
	b	0x10000571c
	adrp	x0, 10 ; 0x10000f000
	add	x0, x0, #0x748 ; literal pool for: "(empty)"
	mov	x1, x19
	bl	0x10000ea00 ; symbol stub for: _fputs
	mov	w0, #0xa
	mov	x1, x19
	bl	0x10000e9f4 ; symbol stub for: _fputc
	adrp	x8, 11 ; 0x100010000
	ldr	x8, [x8, #0x28] ; literal pool symbol address: ___stderrp
	ldr	x1, [x8]
	adrp	x0, 9 ; 0x10000e000
	add	x0, x0, #0xb4d ; literal pool for: "\033[0m"
	bl	0x10000ea00 ; symbol stub for: _fputs
	ldp	x29, x30, [sp, #0x190]
	ldp	x20, x19, [sp, #0x180]
	ldp	x22, x21, [sp, #0x170]
	ldp	x24, x23, [sp, #0x160]
	ldp	x26, x25, [sp, #0x150]
	ldp	x28, x27, [sp, #0x140]
	add	sp, sp, #0x1a0
	ret
	bl	_store_struct.cold.1
	bl	_store_struct.cold.
