    .text
    .globl it49_syslib_handler

it49_syslib_handler:
	
	// Passage de parametres
	pushl %esi
	pushl %edi
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax
	
	// registres de segment en mode protege
	movw $0x18, %dx
	movw %dx, %fs
	movw %dx, %gs
	movw %dx, %es
	movw %dx, %ds
	
	// Appel Traitant
	call it49_syslib_body

	// registres de segment en mode utilisateur
	movw $0x4b, %dx
	movw %dx, %fs
	movw %dx, %gs
	movw %dx, %es
	movw %dx, %ds
	
	// Depilement des paramètres
	addl $24, %esp

	// Retour d'interruption
	iret
