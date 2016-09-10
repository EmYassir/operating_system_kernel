#include "syslib.h"


void console_putbytes(const char *s, int len){
  cons_write(s,len);
}

