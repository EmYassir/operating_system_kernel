#include "cpu.h"
#include "sem.h"
#include "common.h"
#include "console.h"
#include "private_sem.h"
#include "process_handler.h"



/******************************************************************************/
/*                             Variables globales                             */
/******************************************************************************/

// Table des semaphores
static sem sem_table[NB_SEM]; 

static link queue_table[NB_QUEUE]={          // Files de semaphores :
  LIST_HEAD_INIT(queue_table[sem_used]),     //  1) utilises
  LIST_HEAD_INIT(queue_table[sem_free])      //  2) libres
};

/******************************************************************************/
/*                           Gestion id semaphores                            */
/******************************************************************************/

/*
 * Initialisation file semaphores libres
 */
void init_sem_free(void){
  for (int i=0;i<NB_SEM;i++){
    sem_table[i].id=i;
    queue_add(&sem_table[i],
	      &queue_table[sem_free], 
	      sem, 
	      chaining, 
	      id);
  }
}

/*
 * Trouve le semaphore correpondant dans la liste en parametre
 */
static sem* find_sem(link * list, int id) {
  sem* cur; 
  queue_for_each(cur, list, sem, chaining){
    if ((*cur).id==id) {
      return cur;
    }
  }
  return 0;
}

/*
 * acquisition id libre
 */
static int obtain_id(){
  if (queue_empty(&queue_table[sem_free])){
    return -1;
  }
  return (queue_out_bot(&queue_table[sem_free], sem, chaining))->id;
}

/*
 * requisition id
 */
static void free_id(int id){
  sem * cur=find_sem(&queue_table[sem_used], id);
  queue_del(cur, chaining);
  queue_add(&sem_table[cur->id],
	    &queue_table[sem_free], 
	    sem, 
	    chaining, 
	    id);
}




/******************************************************************************/
/*                 Gestion processus bloques sur semaphores                   */
/******************************************************************************/

/*
 * Retourne la valeur du semaphore 
 */
int scount(int id){
  //cli();
  sem * cur=find_sem(&queue_table[sem_used], id);
  if(!cur){
    //sti();
    return -1;
  }  
  //sti();
  return ((cur->value)&0x0000ffff);
}

 
/* 
 * Creation semaphore
 */
int screate(short int count){
  //cli();
  if (count<0){ 
    //sti();
    return -1;
  }
  int id_sem=obtain_id();
  if (id_sem ==-1){
    //sti();
    return -1;
  }  
  sem_table[id_sem].value=(int)count;
  queue_add(&sem_table[id_sem],
	    &queue_table[sem_used], 
	    sem, 
	    chaining, 
	    id);
  //sti();
  return id_sem;
}


/* 
 * Destruction semaphore 
 */
int sdelete(int id){
  
  sem * cur=find_sem(&queue_table[sem_used], id);
  
  // Semaphore invalide
  if (cur==0){
    //sti();
    return -1;
  }
  // Liberation de la liste
  free_id(id);
  unblock_all_sem(id,-3); 
  //sti();
  return 0;
}

 
/*
 * Liberation de tous processus bloque sur le semaphore "id"
 */ 
int sreset(int id, short int count){
  //cli();
  sem * cur=find_sem(&queue_table[sem_used], id);
  // Semaphore invalide
  if (cur==0){
    //sti();
    return -1;
  }
  // Valeur de count negative
  if (count<0){
    //sti();
    return -1;
  }

  cur->value=((int)count&0x0000ffff);

  // Liberation de la liste
  unblock_all_sem(id,-4);
  //sti();
  return 0;
}

/*
 * Liberation du plus prioritaire des processus bloques sur le semaphore "id"
 */
int signal(int id){
  //cli();
  int pid;
  sem * cur=find_sem(&queue_table[sem_used], id);
  // Semaphore invalide
  if (cur==0){
    //sti();
    return -1;
  }

  // Test
  int test = cur->value+1;
  
  // Depassement de capacite
  if(test>SHRT_MAX){
    //sti();
    return -2;
  }

  cur->value=test;
  if(test<=0){
    pid=unblock_sem(id);
    if (getprio(pid)>getprio(getpid())){
      scheduler();
    }
  }
  //sti();  
  return 0;
}

int signal_death(int id){
  //cli();
  sem * cur=find_sem(&queue_table[sem_used], id);
  // Semaphore invalide
  if (cur==0){
    //sti();
    return -1;
  }

  // Test
  int test = cur->value+1;
  
  // Depassement de capacite
  if(test>SHRT_MAX){
    //sti();
    return -2;
  }

  cur->value=test;
  if(test<=0){
    unblock_sem(id);
  }
  //sti();  
  return 0;
}

/*
 * "Signal" atomique
 */
int signaln(int id, short int count){
  // mode maitre masque
  //cli();
  //printf("[signaln] : ");
  //print_list_sem();

  if (count<0){
    //sti();
    return -1;
  }

  sem * cur=find_sem(&queue_table[sem_used], id);
  // Semaphore invalide
  if (cur==0){
    //sti();
    return -1;
  }
  int test = cur->value+(int)count;
  // Depassement de capacite
  if(test>SHRT_MAX){
    //sti();
    return -2;
  }

  cur->value=test;
  unblock_n_sems(id,count);
  //sti();
  return 0;
}



/*
 * wait
 */
int wait(int id){
  //cli();
  sem * cur=find_sem(&queue_table[sem_used], id);
  // Semaphore invalide
  if (cur==0){
    //sti();
    return -1;
  }
  
  int test=cur->value-1;
  // Depassement de capacite du compteur
  if(test < SHRT_MIN){
    //sti();
    return -2;
  }
  cur->value=test;

  // Valeur negative
  if (cur->value<0){
    block_sem(id);

    //  0 si le reveil est consecutif a signal
    // -3 si le reveil est consecutif a sdelete
    // -4 si le reveil est consecutif a sreset
    return  get_awakener();
  }
  //sti();
  return 0;
}

/*
 * try_wait
 */
int try_wait(int id){
  //cli();
  sem * cur=find_sem(&queue_table[sem_used], id);
  // Semaphore invalide
  if (cur==0){
    //sti();
    return -1;
  }

  int test=cur->value-1;

  // depassement de capacite
  if (test<SHRT_MIN){
    //sti();
    return -2;
  }

  // Operation P induisant bloquage
  if(test<0){
    //sti();
    return -3;
  }
  // Operation P possible sans bloquage
  cur->value=test;
  //sti();
  return 0;
}

/******************************************************************************/
/*                           Fonctions auxiliaires                            */
/******************************************************************************/
 
/* Creation mutex */
int mutex_init(){
  return screate(1);
}


/* Reservation section critique */
int mutex_lock(int id){
  return wait(id);
}

/* Test reservation section critique */
int mutex_try_lock(int id){
  return try_wait(id);
}

/* Liberation section critique */
int mutex_unlock(int id){
  return signal(id);
}

/* Destruction mutex */
int mutex_destroy(int id){
  return sdelete(id);
}

/******************************************************************************/
/*                           Liste des semaphores                             */
/******************************************************************************/

/* Liste des semaphores utilises */
void list_sem_info(void){
  printf("id sempahore                       valeur sempahore \n");  
  printf("------------------------------------------------------------\n");
  sem * cur;
  queue_for_each(cur,&queue_table[sem_used], sem, chaining){
    printf("%d                                 %d \n",cur->id,cur->value);
    list_of_sem(cur->id);
  }
  printf("\n");
}
