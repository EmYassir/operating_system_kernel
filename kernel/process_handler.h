#ifndef __PROCESS_HANDLER_H__
#define __PROCESS_HANDLER_H__

#include "common.h"
#include "../shared/queue.h"

/******************************************************************************/
/*                      Initialisation gestion processus                      */
/******************************************************************************/

void init_process_handling(void);


/******************************************************************************/
/*                                Gestion pid                                 */
/******************************************************************************/

// retourne pid processus courant
int getpid(void);

/******************************************************************************/
/*                    Liste de processus bloques sur semaphore                */
/******************************************************************************/
void list_of_sem(int sem);

/******************************************************************************/
/*                              Gestion priorite                              */
/******************************************************************************/

int chprio(int pid, int newprio);

void set_idle_prio();

int getprio(int pid);


/******************************************************************************/
/*                               Ordonnancement                               */
/******************************************************************************/

/* Ordonnanceur */
void scheduler(void);

/******************************************************************************/
/*                             Gestion processus                              */
/******************************************************************************/

// Creation processus
int start(int (*ptfunc)(void *), 
	  unsigned long ssize, 
	  int prio, 
	  const char *name, 
	  void *arg);

// endormissement processus
void sleep(unsigned long time);

// attente temporisee 
void wait_clock(unsigned long clock);


/******************************************************************************/
/*                           Terminaison processus                            */
/******************************************************************************/

/* 
 * Terminaison propre
 */
void exit(int retval);

/*
 * Terminaison brutale
 */
int kill(int pid);

/******************************************************************************/
/*                             Gestion filiation                              */
/******************************************************************************/


int waitpid(int pid, int *retvalp);

/* 
 * Demon d'elimination des zombies qui n'ont pas de père qui les attend
 */
void check_zombies(void);

/******************************************************************************/
/*                            Gestion semaphores                              */
/******************************************************************************/
/* 
 * Retourne une indication sur le mecanisme qui a reveille le processus :
 * 1 )  0 si le reveil est consecutif à signal
 * 2 ) -3 si le reveil est consecutif à sdelete
 * 3 ) -4 si le reveil est consecutif à sreset
 */
int get_awakener();


/* 
 * Bloque un processus sur le semaphore "sem"
 * Retourne son pid en cas de succes
 */
void block_sem(int sem);

/* 
 * Trouve le plus prioritaire des processus bloques par le semaphore 
 * en parametre, le retire de la liste des processus bloques
 * et le place dans la liste des activables.
 * Retourne son pid en cas de succes, -1 sinon.
 */
int unblock_sem(int sem);

/* 
 * Trouve les n plus prioritaire des processus bloques par le semaphore 
 * en parametre, les retire de la liste des processus bloques
 * et les place dans la liste des activables.
 */
void unblock_n_sems(int sem, short int n);

/* 
 * Libere tous les processus bloques sur le semaphore "sem"
 */
void unblock_all_sem(int sem, int wake);

/******************************************************************************/
/*                              Entrees/Sorties                               */
/******************************************************************************/

int wait_on_io();

void cons_echo(int on);

unsigned long cons_read(char *string, unsigned long length);

void signal_end_io(int pid);


/******************************************************************************/
/*                                    SHELL                                   */
/******************************************************************************/

/*
 * Lecture sur interruptions clavier (cas du shell)
 */ 
unsigned long shell_cons_read(char *string, unsigned long length);



/*
 * Liste des processus
 */
void list_ps(void);

/*
 * Prompt du shell
 */
void print_prompt(void);

/*
 * Effacement ecran clavier (cas du shell)
 */ 
void clear(void);

/*
 * DEBUG
 */
void print_list_sem(void);



#endif

