#ifndef __PRIVATE_PROCESS_H__
#define __PRIVATE_PROCESS_H__

#include "queue.h"
/******************************************************************************/
/*                                Structures                                  */
/******************************************************************************/
// Types files de processus
typedef enum _list_type{
  actionable,
  asleep,
  sem_blocked,
  io_blocked,
  waiting_child,
  zombie,
  freepid,
  /**/
  index
}list_type;

#define NB_STATE index


// Structure processus
typedef struct _process{
  char *name;
  int pid;
  int prio;
  int awakener;
  int retvalue;
  int alarm_time;
  int sem_blocked;
  //==========================================
  //    0 |   1  |   2  |   3  |   4  |   5  |
  //==========================================
  //  %esp|  %ebx| %esi |  %edi|  %ebp| %flag|
  int zone_reg[6];
  unsigned long user_stack_size;       // taille en octet de la pile
  void *kernel_stack;                  // pile noyau
  void *user_stack;                    // pointeur sur le debut de la pile utilisateur
  link garbage_collection;

  link chaining;			// chainage sur l'etat
  link children_chaining;		// chainage de la liste des fils (le processus est un fils)
  int father;
  link children;			// liste des enfants de ce processus (le processsus est le père)
  int child_waited;
  list_type state;
}process;

#endif
