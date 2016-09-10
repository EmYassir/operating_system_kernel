#include "net_conf.h"
#include "cpu.h"
#include "pci_conf.h"

//struct netif net_state;

/* Ecrit la valeur val dans le registre reg */
#define net_write_reg(reg, val)                                               \
  do{                                                                         \
    outl(IO_ADDRESS + 0x00, reg);			      		      \
    outl(IO_ADDRESS + 0x04, val);     				              \
  }while(0)

/* Lit la valeur dans le registre reg */
#define net_read_reg(reg)                                                     \
  do{                                                                         \
    outl(IO_ADDRESS + 0x00, reg);	      				      \
    return inl(IO_ADDRESS + 0x04);	      			              \
  }while(0)

void net_init() {

  unsigned long line_content;
  line_content = pci_read_word(network_bus, network_device, 0, 0x3c);

  // Ecrit l'adresse de base des entrees/sorties dans BAR2 et 
  // active le I/O mapping
  line_content = pci_read_word(network_bus, network_device, 0, 0x04);
  pci_write_word(network_bus, network_device, 0, 0x04, line_content|12);
  pci_write_word(network_bus, network_device, 0, 0x18, IO_ADDRESS|1);

  /* Attribue le numero d'IRQ */
  pci_write_word(network_bus, network_device, 0, 0x3c, 
		 (line_content & 0xffff0000) | (NET_PIN << 8) | (NET_IRQ));

  /* Lit l'adresse MAC de la carte */
  // TODO NEXT
}
