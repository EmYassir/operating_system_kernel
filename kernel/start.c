#include "cpu.h"
#include "debugger.h"
#include "debug.h"
#include "common.h"
#include "interruption.h"
#include "interruption_handlers.h"
#include "timer.h"
#include "kbd.h"
#include "sem.h"
#include "console.h"
#include "process_handler.h"
#include "kernel_processes.h"
#include "test.h"
#include "pci_conf.h"
#include "audio.h"

void kernel_init(void) {

	/* PÉRIPHÉRIQUES */

	// Ecran
	initialize_screen();

	// Horloge
	init_clock();

	// Clavier
	kbd_leds(7);

	/* INTERUPTIONS */

	// Interruptions - horloge
	mask_IRQ(0,0);	
	init_handler_IT(32, &it32_clock_handler, 0);

	// Interruptions - clavier
	mask_IRQ(1,0);
	init_handler_IT(33,&it33_kbd_handler, 0);

	// Interruptions - appel système
	init_handler_IT(49, &it49_syslib_handler, 1);

	// Exception instructions protegees
	init_handler_IT(13, &it13_exception_handler, 0);

	// Interruptions - RTC
	init_handler_IT(40, &it40_rtc_handler, 0);

	/* ORDONNANCEUR */

	init_process_handling();

	// Liste des semaphores
	init_sem_free();
       
	/* E/S */

	// Buffers clavier
	init_buffer();

	// Audio
	init_audio();
}

void kernel_start(void)
{
	// Debugger
	//call_debugger();

	// Initialisation
	kernel_init();

	// Creation des nouveaux processus

	start((int (*)(void *))USER_ADR, 0x20000, MAXPRIO, "user_start", 0);

	// Reconnaissance du matériel supporté
//	list_pci_devices(INIT);

	// Affichage des périphériques disponibles
//	list_pci_devices(PRINT);

	// Demarrage du processus par defaut
	idle();
}
