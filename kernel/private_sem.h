#ifndef __PRIVATE_SEM_H__
#define __PRIVATE_SEM_H__

/******************************************************************************/
/*                                Structures                                  */
/******************************************************************************/

// Structure semaphore
typedef struct _sem{ 
  int id;                   // id semaphore
  int value;                // valeur semaphore
  link chaining;            // chainage
}sem;


// Types files de semaphores
typedef enum _list_sem{
  sem_used,
  sem_free,
  entry
}list_sem;

#define NB_QUEUE entry


#endif
