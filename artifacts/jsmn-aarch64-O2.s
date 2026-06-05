	.text
	.file	"jsmn.h"
	.globl	jsmn_parse                      // -- Begin function jsmn_parse
	.p2align	2
	.type	jsmn_parse,@function
jsmn_parse:                             // @jsmn_parse
	.cfi_startproc
// %bb.0:
	stp	x22, x21, [sp, #-32]!           // 16-byte Folded Spill
	.cfi_def_cfa_offset 32
	stp	x20, x19, [sp, #16]             // 16-byte Folded Spill
	.cfi_offset w19, -8
	.cfi_offset w20, -16
	.cfi_offset w21, -24
	.cfi_offset w22, -32
	.cfi_remember_state
	ldp	w6, w15, [x0]
	cmp	x6, x2
	b.hs	.LBB0_104
// %bb.1:
	movi	d0, #0x000000ffffffff
	mov	x16, #9729                      // =0x2601
	mov	x11, #8193                      // =0x2001
	mov	w12, #4113                      // =0x1011
	movk	x16, #4097, lsl #32
	add	x9, x3, #8
	mov	w10, #1                         // =0x1
	movk	x11, #1024, lsl #48
	movk	w12, #5, lsl #16
	mov	w13, #4                         // =0x4
	mov	w14, #8                         // =0x8
	movk	x16, #1024, lsl #48
	mov	w17, #-1                        // =0xffffffff
	mov	w5, w15
	mov	w8, w15
	mov	w18, w6
	b	.LBB0_4
.LBB0_2:                                //   in Loop: Header=BB0_4 Depth=1
	sub	w19, w7, #9
	cmp	w19, #2
	b.hs	.LBB0_84
.LBB0_3:                                //   in Loop: Header=BB0_4 Depth=1
	add	w6, w18, #1
	cmp	x6, x2
	mov	w18, w6
	str	w6, [x0]
	b.hs	.LBB0_105
.LBB0_4:                                // =>This Loop Header: Depth=1
                                        //     Child Loop BB0_55 Depth 2
                                        //     Child Loop BB0_80 Depth 2
                                        //     Child Loop BB0_70 Depth 2
                                        //     Child Loop BB0_17 Depth 2
                                        //     Child Loop BB0_85 Depth 2
	ldrb	w7, [x1, x6]
	cmp	w7, #43
	b.le	.LBB0_9
// %bb.5:                               //   in Loop: Header=BB0_4 Depth=1
	cmp	w7, #92
	b.gt	.LBB0_48
// %bb.6:                               //   in Loop: Header=BB0_4 Depth=1
	cmp	w7, #44
	b.eq	.LBB0_64
// %bb.7:                               //   in Loop: Header=BB0_4 Depth=1
	cmp	w7, #58
	b.eq	.LBB0_74
// %bb.8:                               //   in Loop: Header=BB0_4 Depth=1
	cmp	w7, #91
	mov	w19, w18
	b.eq	.LBB0_59
	b	.LBB0_85
.LBB0_9:                                //   in Loop: Header=BB0_4 Depth=1
	cmp	w7, #12
	b.le	.LBB0_2
// %bb.10:                              //   in Loop: Header=BB0_4 Depth=1
	cmp	w7, #13
	b.eq	.LBB0_3
// %bb.11:                              //   in Loop: Header=BB0_4 Depth=1
	cmp	w7, #32
	b.eq	.LBB0_3
// %bb.12:                              //   in Loop: Header=BB0_4 Depth=1
	cmp	w7, #34
	mov	w19, w18
	b.ne	.LBB0_85
// %bb.13:                              //   in Loop: Header=BB0_4 Depth=1
	add	w6, w18, #1
	cmp	x6, x2
	str	w6, [x0]
	b.hs	.LBB0_112
// %bb.14:                              //   in Loop: Header=BB0_4 Depth=1
	mov	x19, x6
	mov	w7, w6
	b	.LBB0_17
.LBB0_15:                               //   in Loop: Header=BB0_17 Depth=2
	mov	w20, w7
.LBB0_16:                               //   in Loop: Header=BB0_17 Depth=2
	add	w19, w20, #1
	cmp	x19, x2
	mov	w7, w19
	str	w19, [x0]
	b.hs	.LBB0_112
.LBB0_17:                               //   Parent Loop BB0_4 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	ldrb	w19, [x1, x19]
	cmp	w19, #92
	b.eq	.LBB0_20
// %bb.18:                              //   in Loop: Header=BB0_17 Depth=2
	cbz	w19, .LBB0_112
// %bb.19:                              //   in Loop: Header=BB0_17 Depth=2
	cmp	w19, #34
	b.ne	.LBB0_15
	b	.LBB0_98
.LBB0_20:                               //   in Loop: Header=BB0_17 Depth=2
	add	w20, w7, #1
	cmp	x20, x2
	b.hs	.LBB0_15
// %bb.21:                              //   in Loop: Header=BB0_17 Depth=2
	str	w20, [x0]
	ldrb	w19, [x1, x20]
	sub	w21, w19, #98
	cmp	w21, #19
	b.hi	.LBB0_46
// %bb.22:                              //   in Loop: Header=BB0_17 Depth=2
	lsl	w22, w10, w21
	tst	w22, w12
	b.ne	.LBB0_16
// %bb.23:                              //   in Loop: Header=BB0_17 Depth=2
	cmp	w21, #19
	b.ne	.LBB0_46
// %bb.24:                              //   in Loop: Header=BB0_17 Depth=2
	add	w20, w7, #2
	cmp	x20, x2
	mov	w19, w20
	str	w20, [x0]
	b.hs	.LBB0_45
// %bb.25:                              //   in Loop: Header=BB0_17 Depth=2
	ldrb	w20, [x1, x20]
	tst	w20, #0xff
	b.eq	.LBB0_45
// %bb.26:                              //   in Loop: Header=BB0_17 Depth=2
	sub	w19, w20, #48
	and	w19, w19, #0xff
	cmp	w19, #10
	b.lo	.LBB0_29
// %bb.27:                              //   in Loop: Header=BB0_17 Depth=2
	and	w19, w20, #0xff
	sub	w20, w19, #65
	mov	w19, #-2                        // =0xfffffffe
	cmp	w20, #37
	b.hi	.LBB0_113
// %bb.28:                              //   in Loop: Header=BB0_17 Depth=2
	lsl	x20, x10, x20
	tst	x20, #0x3f0000003f
	b.eq	.LBB0_113
.LBB0_29:                               //   in Loop: Header=BB0_17 Depth=2
	add	w19, w7, #3
	cmp	x19, x2
	str	w19, [x0]
	b.hs	.LBB0_45
// %bb.30:                              //   in Loop: Header=BB0_17 Depth=2
	ldrb	w20, [x1, x19]
	tst	w20, #0xff
	b.eq	.LBB0_45
// %bb.31:                              //   in Loop: Header=BB0_17 Depth=2
	sub	w19, w20, #48
	and	w19, w19, #0xff
	cmp	w19, #10
	b.lo	.LBB0_34
// %bb.32:                              //   in Loop: Header=BB0_17 Depth=2
	and	w19, w20, #0xff
	sub	w20, w19, #65
	mov	w19, #-2                        // =0xfffffffe
	cmp	w20, #37
	b.hi	.LBB0_113
// %bb.33:                              //   in Loop: Header=BB0_17 Depth=2
	lsl	x20, x10, x20
	tst	x20, #0x3f0000003f
	b.eq	.LBB0_113
.LBB0_34:                               //   in Loop: Header=BB0_17 Depth=2
	add	w20, w7, #4
	cmp	x20, x2
	mov	w19, w20
	str	w20, [x0]
	b.hs	.LBB0_45
// %bb.35:                              //   in Loop: Header=BB0_17 Depth=2
	ldrb	w20, [x1, x20]
	tst	w20, #0xff
	b.eq	.LBB0_45
// %bb.36:                              //   in Loop: Header=BB0_17 Depth=2
	sub	w19, w20, #48
	and	w19, w19, #0xff
	cmp	w19, #10
	b.lo	.LBB0_39
// %bb.37:                              //   in Loop: Header=BB0_17 Depth=2
	and	w19, w20, #0xff
	sub	w20, w19, #65
	mov	w19, #-2                        // =0xfffffffe
	cmp	w20, #37
	b.hi	.LBB0_113
// %bb.38:                              //   in Loop: Header=BB0_17 Depth=2
	lsl	x20, x10, x20
	tst	x20, #0x3f0000003f
	b.eq	.LBB0_113
.LBB0_39:                               //   in Loop: Header=BB0_17 Depth=2
	add	w20, w7, #5
	cmp	x20, x2
	mov	w19, w20
	str	w20, [x0]
	b.hs	.LBB0_45
// %bb.40:                              //   in Loop: Header=BB0_17 Depth=2
	ldrb	w20, [x1, x20]
	tst	w20, #0xff
	b.eq	.LBB0_45
// %bb.41:                              //   in Loop: Header=BB0_17 Depth=2
	sub	w19, w20, #48
	and	w19, w19, #0xff
	cmp	w19, #10
	b.lo	.LBB0_44
// %bb.42:                              //   in Loop: Header=BB0_17 Depth=2
	and	w19, w20, #0xff
	sub	w20, w19, #65
	mov	w19, #-2                        // =0xfffffffe
	cmp	w20, #37
	b.hi	.LBB0_113
// %bb.43:                              //   in Loop: Header=BB0_17 Depth=2
	lsl	x20, x10, x20
	tst	x20, #0x3f0000003f
	b.eq	.LBB0_113
.LBB0_44:                               //   in Loop: Header=BB0_17 Depth=2
	add	w19, w7, #6
.LBB0_45:                               //   in Loop: Header=BB0_17 Depth=2
	sub	w20, w19, #1
	b	.LBB0_16
.LBB0_46:                               //   in Loop: Header=BB0_17 Depth=2
	sub	w7, w19, #34
	mov	w19, #-2                        // =0xfffffffe
	cmp	w7, #58
	b.hi	.LBB0_113
// %bb.47:                              //   in Loop: Header=BB0_17 Depth=2
	lsl	x7, x10, x7
	tst	x7, x11
	b.ne	.LBB0_16
	b	.LBB0_113
.LBB0_48:                               //   in Loop: Header=BB0_4 Depth=1
	cmp	w7, #93
	b.eq	.LBB0_51
// %bb.49:                              //   in Loop: Header=BB0_4 Depth=1
	cmp	w7, #123
	b.eq	.LBB0_59
// %bb.50:                              //   in Loop: Header=BB0_4 Depth=1
	cmp	w7, #125
	mov	w19, w18
	b.ne	.LBB0_85
.LBB0_51:                               //   in Loop: Header=BB0_4 Depth=1
	cbz	x3, .LBB0_3
// %bb.52:                              //   in Loop: Header=BB0_4 Depth=1
	cmp	w7, #125
	cinc	w7, w10, ne
	subs	w19, w15, #1
	b.mi	.LBB0_75
// %bb.53:                              //   in Loop: Header=BB0_4 Depth=1
	add	x6, x9, w19, uxtw #4
	add	x5, x19, #1
	b	.LBB0_55
.LBB0_54:                               //   in Loop: Header=BB0_55 Depth=2
	subs	x5, x5, #1
	sub	x6, x6, #16
	b.le	.LBB0_114
.LBB0_55:                               //   Parent Loop BB0_4 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	ldur	w19, [x6, #-4]
	cmn	w19, #1
	b.eq	.LBB0_54
// %bb.56:                              //   in Loop: Header=BB0_55 Depth=2
	ldr	w19, [x6]
	cmn	w19, #1
	b.ne	.LBB0_54
// %bb.57:                              //   in Loop: Header=BB0_4 Depth=1
	ldur	w19, [x6, #-8]
	cmp	w19, w7
	b.ne	.LBB0_114
// %bb.58:                              //   in Loop: Header=BB0_4 Depth=1
	add	w7, w18, #1
	sub	w19, w5, #1
	str	w17, [x0, #8]
	str	w7, [x6]
	b	.LBB0_76
.LBB0_59:                               //   in Loop: Header=BB0_4 Depth=1
	add	w8, w8, #1
	cbz	x3, .LBB0_3
// %bb.60:                              //   in Loop: Header=BB0_4 Depth=1
	cmp	w15, w4
	b.hs	.LBB0_117
// %bb.61:                              //   in Loop: Header=BB0_4 Depth=1
	add	x6, x3, w15, uxtw #4
	add	w5, w15, #1
	str	w5, [x0, #4]
	str	d0, [x6, #8]
	ldrsw	x19, [x0, #8]
	cmn	w19, #1
	b.eq	.LBB0_63
// %bb.62:                              //   in Loop: Header=BB0_4 Depth=1
	add	x19, x3, x19, lsl #4
	ldr	w20, [x19, #12]
	add	w20, w20, #1
	str	w20, [x19, #12]
.LBB0_63:                               //   in Loop: Header=BB0_4 Depth=1
	cmp	w7, #123
	str	w15, [x0, #8]
	cinc	w7, w10, ne
	stp	w7, w18, [x6]
	b	.LBB0_96
.LBB0_64:                               //   in Loop: Header=BB0_4 Depth=1
	cbz	x3, .LBB0_3
// %bb.65:                              //   in Loop: Header=BB0_4 Depth=1
	ldrsw	x6, [x0, #8]
	cmn	w6, #1
	b.eq	.LBB0_3
// %bb.66:                              //   in Loop: Header=BB0_4 Depth=1
	lsl	x6, x6, #4
	ldr	w6, [x3, x6]
	sub	w6, w6, #1
	cmp	w6, #2
	b.lo	.LBB0_3
// %bb.67:                              //   in Loop: Header=BB0_4 Depth=1
	subs	w6, w5, #1
	b.mi	.LBB0_3
// %bb.68:                              //   in Loop: Header=BB0_4 Depth=1
	add	x7, x9, w6, uxtw #4
	b	.LBB0_70
.LBB0_69:                               //   in Loop: Header=BB0_70 Depth=2
	cmp	x6, #0
	sub	x6, x6, #1
	sub	x7, x7, #16
	b.le	.LBB0_3
.LBB0_70:                               //   Parent Loop BB0_4 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	ldur	w19, [x7, #-8]
	sub	w19, w19, #1
	cmp	w19, #1
	b.hi	.LBB0_69
// %bb.71:                              //   in Loop: Header=BB0_70 Depth=2
	ldur	w19, [x7, #-4]
	cmn	w19, #1
	b.eq	.LBB0_69
// %bb.72:                              //   in Loop: Header=BB0_70 Depth=2
	ldr	w19, [x7]
	cmn	w19, #1
	b.ne	.LBB0_69
// %bb.73:                              //   in Loop: Header=BB0_4 Depth=1
	str	w6, [x0, #8]
	b	.LBB0_3
.LBB0_74:                               //   in Loop: Header=BB0_4 Depth=1
	sub	w6, w5, #1
	str	w6, [x0, #8]
	b	.LBB0_3
.LBB0_75:                               //   in Loop: Header=BB0_4 Depth=1
	mov	w5, w15
.LBB0_76:                               //   in Loop: Header=BB0_4 Depth=1
	cbz	w5, .LBB0_114
// %bb.77:                              //   in Loop: Header=BB0_4 Depth=1
	tbnz	w19, #31, .LBB0_83
// %bb.78:                              //   in Loop: Header=BB0_4 Depth=1
	add	x5, x9, w19, uxtw #4
	mov	w6, w19
	b	.LBB0_80
.LBB0_79:                               //   in Loop: Header=BB0_80 Depth=2
	cmp	x6, #0
	sub	x6, x6, #1
	sub	x5, x5, #16
	b.le	.LBB0_83
.LBB0_80:                               //   Parent Loop BB0_4 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	ldur	w7, [x5, #-4]
	cmn	w7, #1
	b.eq	.LBB0_79
// %bb.81:                              //   in Loop: Header=BB0_80 Depth=2
	ldr	w7, [x5]
	cmn	w7, #1
	b.ne	.LBB0_79
// %bb.82:                              //   in Loop: Header=BB0_4 Depth=1
	str	w6, [x0, #8]
.LBB0_83:                               //   in Loop: Header=BB0_4 Depth=1
	mov	w5, w15
	b	.LBB0_3
.LBB0_84:                               //   in Loop: Header=BB0_4 Depth=1
	mov	w19, w18
	cbz	w7, .LBB0_105
.LBB0_85:                               //   Parent Loop BB0_4 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	ldrb	w6, [x1, x6]
	cmp	w6, #58
	b.hi	.LBB0_87
// %bb.86:                              //   in Loop: Header=BB0_85 Depth=2
	lsl	x7, x10, x6
	tst	x7, x16
	b.ne	.LBB0_91
.LBB0_87:                               //   in Loop: Header=BB0_85 Depth=2
	cmp	w6, #93
	b.eq	.LBB0_91
// %bb.88:                              //   in Loop: Header=BB0_85 Depth=2
	cmp	w6, #125
	b.eq	.LBB0_91
// %bb.89:                              //   in Loop: Header=BB0_85 Depth=2
	sub	w6, w6, #127
	cmn	w6, #95
	b.lo	.LBB0_116
// %bb.90:                              //   in Loop: Header=BB0_85 Depth=2
	add	w6, w19, #1
	cmp	x6, x2
	mov	w19, w6
	str	w6, [x0]
	b.lo	.LBB0_85
	b	.LBB0_92
.LBB0_91:                               //   in Loop: Header=BB0_4 Depth=1
	mov	w6, w19
.LBB0_92:                               //   in Loop: Header=BB0_4 Depth=1
	cbz	x3, .LBB0_97
// %bb.93:                              //   in Loop: Header=BB0_4 Depth=1
	cmp	w15, w4
	b.hs	.LBB0_118
// %bb.94:                              //   in Loop: Header=BB0_4 Depth=1
	add	w5, w15, #1
	add	x7, x3, w15, uxtw #4
	ldrsw	x15, [x0, #8]
	add	w8, w8, #1
	str	w5, [x0, #4]
	stp	w14, w18, [x7]
	cmn	w15, #1
	sub	w18, w6, #1
	stp	w6, wzr, [x7, #8]
	b.eq	.LBB0_96
// %bb.95:                              //   in Loop: Header=BB0_4 Depth=1
	add	x15, x3, x15, lsl #4
	ldr	w6, [x15, #12]
	add	w6, w6, #1
	str	w6, [x15, #12]
.LBB0_96:                               //   in Loop: Header=BB0_4 Depth=1
	mov	w15, w5
	b	.LBB0_3
.LBB0_97:                               //   in Loop: Header=BB0_4 Depth=1
	sub	w18, w6, #1
	add	w8, w8, #1
	b	.LBB0_3
.LBB0_98:                               //   in Loop: Header=BB0_4 Depth=1
	cbz	x3, .LBB0_103
// %bb.99:                              //   in Loop: Header=BB0_4 Depth=1
	cmp	w5, w4
	b.hs	.LBB0_118
// %bb.100:                             //   in Loop: Header=BB0_4 Depth=1
	ldrsw	x18, [x0, #8]
	add	w15, w5, #1
	add	x5, x3, w5, uxtw #4
	add	w8, w8, #1
	str	w15, [x0, #4]
	cmn	w18, #1
	stp	w13, w6, [x5]
	stp	w7, wzr, [x5, #8]
	b.eq	.LBB0_102
// %bb.101:                             //   in Loop: Header=BB0_4 Depth=1
	add	x18, x3, x18, lsl #4
	ldr	w5, [x18, #12]
	add	w5, w5, #1
	str	w5, [x18, #12]
.LBB0_102:                              //   in Loop: Header=BB0_4 Depth=1
	mov	w5, w15
	mov	w18, w7
	b	.LBB0_3
.LBB0_103:                              //   in Loop: Header=BB0_4 Depth=1
	add	w8, w8, #1
	mov	w18, w7
	b	.LBB0_3
.LBB0_104:
	mov	w8, w15
.LBB0_105:
	cbz	x3, .LBB0_115
// %bb.106:
	subs	w9, w15, #1
	b.mi	.LBB0_115
// %bb.107:
	add	x10, x3, w9, uxtw #4
	add	x9, x9, #1
	add	x10, x10, #8
	b	.LBB0_109
.LBB0_108:                              //   in Loop: Header=BB0_109 Depth=1
	subs	x9, x9, #1
	sub	x10, x10, #16
	b.le	.LBB0_115
.LBB0_109:                              // =>This Inner Loop Header: Depth=1
	ldur	w11, [x10, #-4]
	cmn	w11, #1
	b.eq	.LBB0_108
// %bb.110:                             //   in Loop: Header=BB0_109 Depth=1
	ldr	w11, [x10]
	cmn	w11, #1
	b.ne	.LBB0_108
// %bb.111:
	mov	w8, #-3                         // =0xfffffffd
	b	.LBB0_115
.LBB0_112:
	mov	w19, #-3                        // =0xfffffffd
.LBB0_113:
	str	w18, [x0]
	mov	w8, w19
	b	.LBB0_115
.LBB0_114:
	mov	w8, #-2                         // =0xfffffffe
.LBB0_115:
	ldp	x20, x19, [sp, #16]             // 16-byte Folded Reload
	mov	w0, w8
	ldp	x22, x21, [sp], #32             // 16-byte Folded Reload
	.cfi_def_cfa_offset 0
	.cfi_restore w19
	.cfi_restore w20
	.cfi_restore w21
	.cfi_restore w22
	ret
.LBB0_116:
	.cfi_restore_state
	mov	w19, #-2                        // =0xfffffffe
	b	.LBB0_113
.LBB0_117:
	mov	w8, #-1                         // =0xffffffff
	b	.LBB0_115
.LBB0_118:
	mov	w19, #-1                        // =0xffffffff
	b	.LBB0_113
.Lfunc_end0:
	.size	jsmn_parse, .Lfunc_end0-jsmn_parse
	.cfi_endproc
                                        // -- End function
	.globl	jsmn_init                       // -- Begin function jsmn_init
	.p2align	2
	.type	jsmn_init,@function
jsmn_init:                              // @jsmn_init
	.cfi_startproc
// %bb.0:
	mov	w8, #-1                         // =0xffffffff
	str	xzr, [x0]
	str	w8, [x0, #8]
	ret
.Lfunc_end1:
	.size	jsmn_init, .Lfunc_end1-jsmn_init
	.cfi_endproc
                                        // -- End function
	.ident	"Ubuntu clang version 18.1.3 (1ubuntu1)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
