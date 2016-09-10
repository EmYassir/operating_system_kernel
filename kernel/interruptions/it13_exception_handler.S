    .text
    .globl it13_exception_handler

it13_exception_handler:
	
	// Sauvegarde des registres
	pushl %eax
	pushl %ecx
	pushl %edx
	
	// Acquittement interruption
//	movb $0x20, %al
//	outb %al, $0x20
	
	// registres de segment en mode protege
	movw $0x18, %dx
	movw %dx, %fs
	movw %dx, %gs
	movw %dx, %es
	movw %dx, %ds
	
	// Appel Traitant
	call it13_exception_body

	// registres de segment en mode utilisateur
	movw $0x4b, %dx
	movw %dx, %fs
	movw %dx, %gs
	movw %dx, %es
	movw %dx, %ds
		
	// Restauration des registres
	popl %edx
	popl %ecx
	popl %eax

	// Retour d'interruption
	iret
