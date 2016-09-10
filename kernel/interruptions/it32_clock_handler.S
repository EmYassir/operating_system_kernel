#include "port.h"

    .text
    .globl it32_clock_handler
 
it32_clock_handler:

	// Sauvegarde des registres
	pushl %eax
	pushl %ecx
	pushl %edx

	// Acquittement interruption
	movb $0x20, %al
	outb %al, $PIC1_CMD

	// registres de segment en mode protege
	movw $0x18, %dx
	movw %dx, %fs
	movw %dx, %gs
	movw %dx, %es
	movw %dx, %ds
	
	// Appel Traitant
	call it32_clock_body

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
