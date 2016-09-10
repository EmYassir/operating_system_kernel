	.text
	.globl goto_user_mode
goto_user_mode:
	movw $0x4b, %dx
	movw %dx, %es
	movw %dx, %fs
	movw %dx, %gs
	movw %dx, %ds

	iret
