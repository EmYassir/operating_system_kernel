#ifndef __PRIVATE_KBD_H__
#define __PRIVATE_KBD_H__

#include "queue.h"
#define BUFFER_SIZE 100

// Structure case de buffer
typedef struct _frame{
  int id;                   // identifiant
  char val;                 // valeur caractere reçu
  link chaining;            // chainage
  unsigned long arrival;    // priorite (temps d'arrivee)
}frame;


// Types files
typedef enum _num_queue{
  kbd_used,
  kbd_free,
  type_queue
}num_queue;

#define NB_QUEUES type_queue

#endif
