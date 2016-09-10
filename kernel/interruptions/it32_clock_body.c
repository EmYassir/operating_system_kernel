#include "common.h"
#include "timer.h"
#include "process_handler.h"

void it32_clock_body(void)
{
	// compte le nombre d'appel au handler, divise la frequence
	static unsigned int sched_ticks = 0;

	// gestion de l'horloge
	increment_clock();

	// gestion de l'ordonnanceur
	sched_ticks++;

	if (sched_ticks == CLOCKFREQ / SCHEDFREQ) {
		sched_ticks = 0;
		scheduler();
	}
}
