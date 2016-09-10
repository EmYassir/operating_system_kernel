#ifndef __COMMON_H__
#define __COMMON_H__

#define CLOCKFREQ		1000		// Frequence horloge
#define SCHEDFREQ		50		// Frequence ordonnanceur
#define QUARTZ			0x1234DD	// Frequence quartz systeme
#define NB_PROCESS 		100		// Nombre maximal processus
#define MAX_SIZE_PROCESS_NAME	16		// taille max du nom d'un processus
#define NB_SEM			100		// Nombre maximal de semaphores (10000)
#define MAXPRIO			256		// Priorite maximal
#define SHRT_MAX		32767           // Max short int
#define SHRT_MIN		-32767		// Min short int 
#define KERNEL_STACK_SIZE	0x20000		// Taille pile noyau
#define USER_ADR		0x1000000
#define PROCESS_RETURN_ADR	0x1000009
#define KILL_RETVAL		0
#define NB_MBOXES               10              // Nombre de mailboxes lwIP



/* SHELL */

#define PROMPT "root@root-anonymOS:~$ "
#define PROMPT_SIZE (strlen(PROMPT))
#define CMD_MAX_SIZE 40

#endif
