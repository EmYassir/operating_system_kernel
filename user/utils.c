#include "../kernel/common.h"
#include "syslib.h"

/* 
 * Endormissement processus
 */
void sleep(unsigned long time) {
	wait_clock(time*CLOCKFREQ);
}
