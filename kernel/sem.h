#ifndef __SEM_H__
#define __SEM_H__


#include "queue.h"





/******************************************************************************/
/*                           Gestion id semaphores                            */
/******************************************************************************/

// Initialisation file semaphores libres
void init_sem_free(void);



/******************************************************************************/
/*                             Gestion semaphores                             */
/******************************************************************************/
int scount(int id);
int screate(short int count);
int sdelete(int id);
int signal(int id);
int signal_death(int id); // appel seulement dans kill
int signaln(int id, short int count);
int wait(int id);
int try_wait(int id);
int sreset(int id, short int count);

/******************************************************************************/
/*                           Fonctions auxiliaires                            */
/******************************************************************************/
 
/* Creation mutex */
int mutex_init();

/* Reservation section critique */
int mutex_lock(int id);

/* Test reservation section critique */
int mutex_try_lock(int id);

/* Liberation section critique */
int mutex_unlock(int id);

/* Destruction mutex */
int mutex_destroy(int id);


/******************************************************************************/
/*                           Liste des semaphores                             */
/******************************************************************************/

/* Liste des semaphores utilises */
void list_sem_info(void);

#endif
