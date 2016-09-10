#include "pci_conf.h"
#include "cpu.h"
#include "debug.h"

extern char* vendor(unsigned short vendor_id);
extern char* model(unsigned long id);
extern char* device_type(unsigned short device_func);

unsigned short network_bus = 0xffff;
unsigned short network_device = 0xffff;
unsigned long network_io_address = 0xffff;

unsigned long pci_read_word(unsigned short bus, unsigned short device, 
			   unsigned short func, unsigned short reg) {

  unsigned long address;
  unsigned long lbus = (unsigned long)bus;
  unsigned long ldevice = (unsigned long)device;
  unsigned long lfunc = (unsigned long)func;
  
  address = (unsigned long)((lbus << 16) | (ldevice << 11) |
			    (lfunc << 8) | (reg & 0xfc) | 0x80000000);
  outl(address, 0xCF8);
  return inl(0xCFC);

}

void pci_write_word(unsigned short bus, unsigned short device, 
		   unsigned short func, unsigned short reg, 
		   unsigned long data) {

  unsigned long address;
  unsigned long lbus = (unsigned long)bus;
  unsigned long ldevice = (unsigned long)device;
  unsigned long lfunc = (unsigned long)func;
  
  address = (unsigned long)((lbus << 16) | (ldevice << 11) |
			    (lfunc << 8) | (reg & 0xfc) | 0x80000000);
  outl(address, 0xCF8);
  return outl(data, 0xCFC);

}

void test_network_card(unsigned short bus, unsigned short device) {

 
  // Vérification de la compatibilité de la carte avec le pilote à intégrer :
 
  unsigned long device_model = pci_read_word(bus, device, 0, 0x00);
  unsigned short device_id = (unsigned short)(device_model>>16);
 
  if ((unsigned short)(device_model&0xffff)==0x8086) {
    // vendor : Intel
    
    int COMPATIBLE = (device_id==0x100E || device_id==0x100F || 
                      device_id==0x1010 || device_id==0x1011 || 
                      device_id==0x1012 || device_id==0x1013 ||
		      device_id==0x1015 || device_id==0x1016 || 
                      device_id==0x1017 || device_id==0x1018 || 
                      device_id==0x1019 || device_id==0x101A || 
		      device_id==0x101D || device_id==0x1026 || 
                      device_id==0x1027 || device_id==0x1028 || 
                      device_id==0x1076 || device_id==0x1077 || 
		      device_id==0x1078 || device_id==0x1079 || 
                      device_id==0x107A || device_id==0x107B || 
		      device_id==0x1107 || device_id==0x1112 );
                     
    if (COMPATIBLE) {
      network_bus = bus;
      network_device = device;
      
      /*
      for(int i=0; i<6; i++) {
	// Cherche l'adresse de base des registres d'entree/sortie
	device_model = pci_read_word(network_bus, network_device, 0, 0x10+4*i);
	if (device_model & 1) {
	  network_io_address = device_model & 0xfffffff8;
	  break;
	}
      }
      */
    }
  }
}

void list_pci_devices(lspci behaviour) {
  unsigned long device_model;
  unsigned short device_func;
  char *d_vendor, *d_type, *d_model;
  int i=0;
  for(int bus=0; bus<256; bus++) {
    for(int device=0; device<32; device++) {
      device_model = pci_read_word(bus, device, 0, 0x00);
      if ((device_model & 0xffff)!=0xffff) { 
	// vendor=0xffff when device doesn't exist
	i++;
	device_func = (unsigned short)(pci_read_word(bus, device, 0, 0x04)>>16);
	d_type = device_type(device_func);
	d_vendor = vendor((unsigned short)(device_model&0xffff));
	d_model = model(device_model);
	  
	if (behaviour==PRINT) {

	  if (d_type==0) {
	    if (d_vendor==0) {
	      printf("device %d : %d %d %d\n", i, device_func, 
		     (unsigned short)(device_model&0xffff), 
		     (unsigned short)(device_model>>16));
	    } else {
	      if (d_model==0) {
		printf("device %d : %d %s %d\n", i, device_func, 
		       vendor((unsigned short)(device_model&0xffff)),
		       (unsigned short)(device_model>>16));
	      } else {
		printf("device %d : %d %s %s\n", i, device_func, 
		       vendor((unsigned short)(device_model&0xffff)),
		       model(device_model));
	      }
	    }
	  } else {
	    if (d_vendor==0) {
	      printf("device %d : %s%d %d\n", i, device_type(device_func), 
		     (unsigned short)(device_model&0xffff), 
		     (unsigned short)(device_model>>16));
	    } else {
	      if (d_model==0) {
		printf("device %d : %s%s %d\n", i, device_type(device_func), 
		       vendor((unsigned short)(device_model&0xffff)),
		       (unsigned short)(device_model>>16));
	      } else {
		printf("device %d : %s%s %s\n", i, device_type(device_func), 
		       vendor((unsigned short)(device_model&0xffff)),
		       model(device_model));
	      }
	    }
	  }

	} else if (behaviour==INIT) {
	  test_network_card(bus,device);
	  //printf("header=%d\n", (unsigned short)((pci_read_word(bus, device, 0, 0x0C)>>16)%256));
	}
      }
    }
  }
  /* Affiche les contenus des registres BAR de la carte réseau */
  /*
    device_model = pci_read_word(network_bus, network_device, 0, 0x00);
    device_func = (unsigned short)(pci_read_word(network_bus, network_device, 
    0, 0x04)>>16);
    printf("network device compatible : %s%s %s\n", device_type(device_func), 
    vendor((unsigned short)(device_model&0xffff)),
    model(device_model));
    device_model = pci_read_word(network_bus, network_device, 0, 0x3c);
    printf("Last line : %ld\n",device_model & 0xffff);
    for(int i=0; i<6; i++) {
    device_model = pci_read_word(network_bus, network_device, 0, 0x10+4*i);
    printf("BAR%d : %ld\n",i,device_model);
    }
  */
}


 



