#include "port.h"

    .text
    .globl it40_rtc_handler
 
it40_rtc_handler:

	// Sauvegarde des registres
	pushl %eax
	pushl %ecx
	pushl %edx

	// registres de segment en mode protege
	movw $0x18, %dx
	movw %dx, %fs
	movw %dx, %gs
	movw %dx, %es
	movw %dx, %ds
	
	// Appel Traitant
	call it40_rtc_body

	// Acquittement interruption
	movb $0x20, %al
	outb %al, $PIC2_CMD	// esclave
	outb %al, $PIC1_CMD	// maître

	// registres de segment en mode utilisateur
	movw $0x4b, %dx
	movw %dx, %fs
	movw %dx, %gs
	movw %dx, %es
	movw %dx, %ds

	// Lecture du registre C pour autoriser prochaine IRQ
	movb $0x0C, %al			// NMI masquée car bit[7] = 0
	outb %al, $CMOS_ADDRESS
	inb $CMOS_DATA

	// Restauration des registres
	popl %edx
	popl %ecx
	popl %eax

	// Retour d'interruption
	iret
