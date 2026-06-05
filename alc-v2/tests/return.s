	.section	__TEXT,__text,regular,pure_instructions

	.globl _main
	.p2align 2
_main:
	mov w0, #0
	b .main.ret
.main.ret:
	ret
