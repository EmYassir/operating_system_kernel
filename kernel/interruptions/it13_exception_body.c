#include "common.h"
#include "process_handler.h"
#include "string.h"

int it13_exception_body(void) {
  printf("ERROR : Invalid instruction\n");
  exit(0);
}
