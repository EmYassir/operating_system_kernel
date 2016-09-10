#include "cpu.h"
#include "kernel_processes.h"
#include "process_handler.h"
#include "debugger.h"

/*
 * Processus idle noyau.
 */
int idle()
{
  for (;;) {
    sti();
    hlt();
    cli();
  }
}

/*
 * Démon pour suppression des zombies
 */
int daemon_Zombieslayer() {
  while(1) {
    check_zombies();
    sleep(10);
  }
}
