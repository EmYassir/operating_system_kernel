char* vendor(unsigned short vendor_id) {

  switch (vendor_id) {

  case 0x1013 :
    return "Cirrus Logic";

  case 0x1022 :
    return "Advanced Micro Devices";

  case 0x10de :
    return "NVIDIA Corporation";

  case 0x10ec :
    return "Realtek Semiconductor Corp.";

  case 0x8086 :
    return "Intel Corporation";

  case 0x8087 :
    return "Intel";

  case 0x80ee :
    return "Oracle Corporation";

  default :
    return (char*)0;

  }
}

char* model(unsigned long id) {

  unsigned short vendor_id = (unsigned short)(id & 0xffff);
  unsigned short device_id = (unsigned short)(id >> 16);

  switch (vendor_id) {

  case 0x1013 :
    // Cirrus Logic

    switch (device_id) {
      
    case 0x00B8 :
      return "64-bit VisualMedia Accelerator CL-GD5446";

    default :
      return (char*)0;

    }

  case 0x10ec :
    // Realtek Semiconductor Corp.

    switch (device_id) {

    case 0x8139 :
      return "Realtek RTL8139 Family PCI Fast Ethernet NIC";
      
    default :
      return (char*)0;

    }

  case 0x1022 :
    // Advanced Micro Devices 
    
    switch (device_id) {

    case 0x2000 :
      return "PCnet LANCE PCI Ethernet Controller Am79C970/1/2/3/5/6";
      
    default :
      return (char*)0;

    }

  case 0x8086 :
    // Intel Corporation

    switch (device_id) {

    case 0x100e :
      return "Intel Pro 1000/MT 02000";

    case 0x100f :
      return "Gigabit Ethernet Controller (copper) 82545EM";

    case 0x101d :
      return "Gigabit Ethernet Controller (copper) 82546EB";

    case 0x1237 :
      return "PCI & Memory 82440LX/EX";

    case 0x7000 :
      return "PIIX3 PCI-to-ISA Bridge (Triton II) 82371SB";

    case 0x7113 :
      return "PIIX4/4E/4M Power Management Controller 82371AB/EB/MB";

    default :
      return (char*)0;

    }

    case 0x80ee :
      // Oracle Corporation

      switch (device_id) {

	//case 

      default :
	return (char*)0;

      }

  default :
    return (char*)0;

  }
}

char* device_type(unsigned short device_func) {
  // Placer un espace après le nom : "Ethernet Controller "

  switch (device_func) {

  case 0x0000 :
    return "";
    
  case 0x0200 :
    return "Ethernet Controller ";

  case 0x0230 :
    return "ATM Controller ";

  case 0x0280 :
    return "Network Controller ";

  default :
    return (char*)0;
    
  }
}
