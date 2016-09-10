#include "segment.h"
#include "cpu.h"
#include "port.h"

#define IDT_ADDRESS 0x1000

void init_handler_IT(int num_IT, void (*handler)(void), int user) {

	// on mulptiplie par 8 car 2 mots de 4 octets par descripteur
	unsigned long *addr_table_IT = (unsigned long *)(IDT_ADDRESS + 8*num_IT);

	*(addr_table_IT++) =
		(KERNEL_CS << 16) | ((unsigned long)handler & 0xFFFF);
	if(user==0){
	  *addr_table_IT = ((unsigned long)handler & 0xFFFF0000) | 0x8E00;
	}
	else{
	  *addr_table_IT = ((unsigned long)handler & 0xFFFF0000) | 0xEE00;
	}
}

void mask_IRQ(int num_IRQ, int mask){
	unsigned short int port;
	unsigned char new_mask;

	// on détermine le PIC concerné
	if (num_IRQ < 8) {
		port = PIC1_DATA;
	} else {
		port = PIC2_DATA;
		num_IRQ -= 8;
	}

	// on force le bit
	new_mask = inb(port);

	if (mask) {
		new_mask |= (1 << num_IRQ);
	} else {
		new_mask &= ~(1 << num_IRQ);
	}

	outb(new_mask, port); 
}
