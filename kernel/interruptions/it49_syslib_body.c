#include "common.h"
#include "console.h"
#include "sem.h"
#include "timer.h"
#include "process_handler.h"
#include "kbd.h"
#include "string.h"
#include "audio.h"

#define UNIMPLEMENTED 0

static int on =1;

int it49_syslib_body(int call, int par1, int par2, int par3, int par4, int par5) {
  switch(call) {
  case 1 :
    return chprio(par1,par2);
    break;
  case 2 :
    // Test sécurité
    if ((unsigned long)par1 < USER_ADR) {
	    return -1;
    }
    if ((unsigned long)par2 < USER_ADR) {
	    return -1;
    }
    clock_settings((unsigned long *)par1, (unsigned long *)par2);
    break;
  case 3 :
    cons_echo(par1);
    break;
  case 4 :
    // Test sécurité
    if ((unsigned long)par1 < USER_ADR) {
	    return 0;
    }
    return (int)(cons_read((char *)par1, (unsigned long)par2));
    break;
  case 5 :
    // Test sécurité
    if ((unsigned long)par1 < USER_ADR) {
	    return -1;
    }
    console_putbytes((char*)par1, par2);
    break;
  case 6 :
    return current_clock();
    break;
  case 7 :
    exit(par1);	
    break;
  case 8 :
    return getpid();
    break;
  case 9 :
    return getprio(par1);
    break;
  case 10 :
    return kill(par1);
    break;
  case 11 :
    return scount(par1);
    break; 
  case 12 :
    return screate((short int) par1);
    break;
  case 13 :
    return sdelete(par1);
    break;
  case 14 :
    return sreset(par1,(short int) par2);
    break;
  case 15 :
    return signal(par1);
    break; 
  case 16 :
    return signaln(par1, (short int)par2);
    break;  
  case 17 :
    // Test sécurité
    if ((unsigned long)par4 < USER_ADR) {
	    return -1;
    }
    return start((int (*)(void *))par1, par2, par3,
		 (const char *)par4, (void *)par5);
    break;
  case 20 :
    wait_clock((unsigned long)par1);
    break;
  case 21 :
    // Test sécurité
    if ((par2 != 0) && ((unsigned long)par2 < USER_ADR)) {
	    return -1;
    }
    return waitpid(par1, (int *)par2);
    break;
 case 22 :
    return wait(par1);
    break;
 case 23 :
    return try_wait(par1);
    break;
  case 24 :
    on = (on+1)%2;
    cons_echo(on);
    return 0;
    break;
  case 25 :
    exit(0);
    break;
  case 26 :
    list_ps();
    return 0;
    break;
  case 27 :
    list_sem_info();
    return 0;
    break;
  case 28 :
    set_col_min(par1);
    return 0;
    break;
  case 29 :
    return 0;
    break;
  case 30 :
    update_buffer();
    return 0;
    break;
  case 31 :
    return 0;
    break;
  case 32 :
    return 0;
    break;
  case 33 :
    // Prompt
    print_prompt();
    return 0;
    break; 
  case 34 :
    // Test sécurité
    if ((unsigned long)par1 < USER_ADR) {
	    return 0;
    }
    // Shell_cons_red
    return (int)shell_cons_read((char *)par1,(unsigned long)par2);
    break; 
  case 35 :
    // effacement d'ecran
    clear();
    return 0;
    break; 
  case 36 :
    // Test sécurité
    if ((unsigned long)par1 < USER_ADR) {
	    return -1;
    }
    play_music((track *)par1);
    break;
  default :
    printf("SYSCALL ERROR : %d, %d, %d, %d, %d, %d\n",call, par1, par2, par3, par4, par5);
    assert(UNIMPLEMENTED);
  }
  return 0;
}
