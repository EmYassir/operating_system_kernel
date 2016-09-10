#include "console.h"


void console_putbytes(const char *s, int len){
  for (int i =0;i<len;i++){
    handle_char(s[i]);
  }
}

