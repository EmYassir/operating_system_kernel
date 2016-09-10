/* Fichier dédié à la configuration des cartes PCI */

extern unsigned short network_bus;
extern unsigned short network_device;
extern unsigned long network_io_address;

typedef enum {
  INIT,
  PRINT
}lspci;

/* Si behaviour vaut INIT, il cherche les cartes compatibles avec 
 * les périphériques, si PRINT, il affiche les périphériques connectés,
 * comme la commande lspci */
void list_pci_devices(lspci behaviour);

unsigned long pci_read_word(unsigned short bus, unsigned short device, 
			    unsigned short func, unsigned short reg);

void pci_write_word(unsigned short bus, unsigned short device, 
		    unsigned short func, unsigned short reg, 
		    unsigned long data);
