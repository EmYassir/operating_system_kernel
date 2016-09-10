#include "cpu.h"
#include "segment.h"
#include "processor_structs.h"
#include "mem.h"
#include "user_stack_mem.h"
#include "string.h"
#include "common.h"
#include "private_process.h"
#include "process_handler.h"
#include "console.h"
#include "timer.h"
#include "sem.h"
#include "kbd.h"
#include "test.h"
#include "debugger.h"
#include "kernel_processes.h"

/******************************************************************************/
/*                             Variables globales                             */
/******************************************************************************/

static int cur_pid = 0;       // pid du processus courant
static int read_mutex = 1;    // mutex pour utilisation de cons_read()

static process process_table[NB_PROCESS+1];

static link queue_table[NB_STATE]={          // Files de processus :
	LIST_HEAD_INIT(queue_table[actionable]),   // 1) activables
	LIST_HEAD_INIT(queue_table[asleep]),       // 2) endormis
	LIST_HEAD_INIT(queue_table[sem_blocked]),  // 3) bloques sur semaphore
	LIST_HEAD_INIT(queue_table[io_blocked]),   // 4) bloques sur entree/sortie
	LIST_HEAD_INIT(queue_table[waiting_child]),// 5) bloques en attente d'un fils
	LIST_HEAD_INIT(queue_table[zombie]),       // 6) zombies 
	LIST_HEAD_INIT(queue_table[freepid])       // 7) pid libres
};

// libération de la mémoire
// ------------------ attention, accès délicat en SMP ------------------
#define PID_GARBAGE NB_PROCESS	// pid du processus de collection de mémoire
static link garbage_collection_list = LIST_HEAD_INIT(garbage_collection_list);	// liste des processus dont il faut libérer la mémoire

/******************************************************************************/
/*                         Fonctions externes utiles                          */
/******************************************************************************/

/* 
 * Changement de contexte
 */
extern void ctx_sw(int* , int*);


/******************************************************************************/
/*                                Gestion pid                                 */
/******************************************************************************/

/*
 * Initialisation file pid libres
 */ 
static void init_free_list(void){
	for (int i=0;i<NB_PROCESS;i++){
		process_table[i].pid=i;
		process_table[i].state=freepid;
		process_table[i].child_waited=0;
		process_table[i].awakener=-1;
		process_table[i].sem_blocked=-1;
		// Initialisation champs de liste
		process_table[i].chaining.next=0;
		process_table[i].chaining.prev=0;
		process_table[i].children_chaining.next=0;
		process_table[i].children_chaining.prev=0;
		queue_add(&process_table[i],&queue_table[freepid],process,chaining,pid);
	}
}


/*
 * Acquisition pid
 */
static int obtain_pid(void){
	if (queue_empty(&queue_table[freepid])){
		return -1;
	}
	return (queue_out_bot(&queue_table[freepid], process, chaining))->pid;
}

/* 
 * Requisition pid.
 * ---
 * Précondition :
 * Le pid ne doit être rendu que lorque
 *  + le processus n'a pas de père, ou son père a fait un waitpid
 *  + la mémoire a été récupérée
 */
static void free_pid(int pid){
	// suppression du nom
	mem_free(process_table[pid].name, strnlen(process_table[pid].name, MAX_SIZE_PROCESS_NAME) + 1);

	// MaJ structures sémaphores
	process_table[pid].awakener=-1;
	process_table[pid].sem_blocked=-1;

	// MaJ structures ordonnanceur
	process_table[pid].state = freepid;
	queue_add(&process_table[pid],
			&queue_table[freepid], 
			process, 
			chaining, 
			pid);
}

/*
 * Recherche par pid d'un processus dans une liste
 */
static process* find_pid(link * list, int pid) 
{
  process* cur;
  queue_for_each(cur, list, process, chaining){
    if ((*cur).pid==pid) {
      return cur;
    }
  }

  return 0;
}



/*
 * Recherche d'un processus dans une liste de fils
 */
static process* find_pid_children(link * list, int pid) 
{
	process* cur;
	queue_for_each(cur, list, process, children_chaining)
	{
		if ((*cur).pid==pid) {
			return cur;
		}
	}
	return 0;
}

/* 
 * Retourne le pid du processus courant
 */
int getpid(void){
	return cur_pid;
}

/******************************************************************************/
/*                    Liste de processus bloques sur semaphore                */
/******************************************************************************/
void list_of_sem(int sem){
  process * cur;
  queue_for_each(cur,&queue_table[sem_blocked], process, chaining){
    printf("%d : %s                     \n",cur->pid,cur->name);
  }
}

/******************************************************************************/
/*                              Gestion priorite                              */
/******************************************************************************/

/* 
 * Changement priorite
 */
int chprio(int pid, int newprio){
	// pid invalide
	if (pid < 0 || pid >= NB_PROCESS) {
		PRINT_DEBUG(TST_PRIO, 3, "------ chprio BAD_PID(%i) error\n", pid);
		return -1;
	}

	// pid inutilisé
	if (process_table[pid].state == freepid) {
		PRINT_DEBUG(TST_PRIO, 3, "------ chprio FREE_PID(%i) error\n", pid);
		return -1;
	}

	// processus zombie
	if (process_table[pid].state == zombie) {
		PRINT_DEBUG(TST_PRIO, 3, "------ chprio FREE_PID(%i) error\n", pid);
		return -1;
	}

	// mauvaise priorité
	if (newprio < 1 || newprio > MAXPRIO) {
		PRINT_DEBUG(TST_PRIO, 3, "------ chprio BAD_PRIO(%i) error\n", newprio);
		return -1;
	}

	// pid valide
	int old = process_table[pid].prio;
	process_table[pid].prio = newprio;

	// changement de place dans la file -- TODO ne faire que si prio change vraiment
	if (pid != cur_pid) {
		process * cur = &process_table[pid];
		queue_del(cur, chaining);
		queue_add(&process_table[pid], &queue_table[cur->state], 
				process, chaining, prio); 
	}

	// réordonnacement si nécessaire
	if (pid != cur_pid && newprio > getprio(cur_pid)) {
		// le processus se retrouve plus prioritaire que le processus courant
		scheduler();
	} else if (pid == cur_pid && newprio < old) { // TODO -- check specs
		// le processus courant devient moins prioritaire qu'un autre
		scheduler();
	}


	return old;
}

/* 
 * Consultation priorite
 */
int getprio(int pid){
	// pid invalide
	if (pid < 0 || pid >= NB_PROCESS) {
		PRINT_DEBUG(TST_PRIO, 3, 
				"------ getprio BAD_PID(%i) error\n", 
				pid);
		return -1;
	}

	// pid inutilisé
	if (process_table[pid].state == freepid) {
		PRINT_DEBUG(TST_PRIO, 3, 
				"------ getprio FREE_PID(%i) error\n", 
				pid);
		return -1;
	}

	// processus zombie
	if (process_table[pid].state == zombie) {
		PRINT_DEBUG(TST_PRIO, 3, 
				"------ getprio ZOMBIE(%i) error\n", 
				pid);
		return -1;
	}

	// pid valide
	PRINT_DEBUG(TST_PRIO, 3, 
			"------ getprio : %i prio is %i\n", 
			pid, 
			process_table[pid].prio);
	return process_table[pid].prio;
}



/******************************************************************************/
/*                             Gestion processus                              */
/******************************************************************************/

/*
 * Creation processus :
 *
 * Contexte de pile mode kernel :
 *                                 |                     | 
 * KERNEL_STACK_SIZE - 6           |_____________________|  <-- %esp
 *                                 |                     |
 *                                 |  @goto_user_mode    |
 * KERNEL_STACK_SIZE - 5           |_____________________| 
 *                                 |                     |
 *                                 |      @ptfonc        | 
 * KERNEL_STACK_SIZE - 4           |_____________________|      
 *                                 |                     |
 *               		   |           CS        |
 * KERNEL_STACK_SIZE - 3           |_____________________|
 *	 	                   |                     |
 *               	 	   |       eflags        | 
 * KERNEL_STACK_SIZE - 2           |_____________________| 
 *	 	                   |                     |
 *               	 	   |          esp        |
 * KERNEL_STACK_SIZE - 1           |_____________________| 
 * 	        	           |                     |
 *              	           |           SS        | 
 * KERNEL_STACK_SIZE               |_____________________|  <-- esp0
 *
 * 
 *
 * Contexte de pile mode user :
 *
 *	 	                   |                     | 
 *   ssize                         |_____________________|
 *	 	                   |                     |
 *	 	                   |   &process_return   |
 *   ssize + 1                     |_____________________|
 *	 	                   |                     |
 *	 	                   |   arg               | 
 *   ssize + 2                     |_____________________|
 */


extern void goto_user_mode(void);

int start(int (*ptfunc)(void *), 
		unsigned long ssize, 
		int prio, 
		const char *name, 
		void *arg) {
	int new_pid;

	PRINT_DEBUG(TST_START, 3, 
			">>>>>> %i enter start(%p, %lu, %i, %s, %p)\n", 
			cur_pid, ptfunc, 
			ssize, prio, 
			name, arg);

	/*
	 * Verification des arguments.
	 */

	// Verification de la taille de pile
	if (ssize > 0xffffffff - 2*sizeof(int)) {
	  PRINT_DEBUG(TST_START, 
		      3, 
		      "<<<<<< %i exit start, BAD_SSIZE error\n", 
		      cur_pid);
	  return -1;
	}

	// Verification de la priorite.
	if (prio < 1 || prio > MAXPRIO) {
		// echec mauvais argument prio
		PRINT_DEBUG(TST_START, 
				3, 
				"<<<<<< %i exit start, BAD_PRIO error\n", 
				cur_pid);
		return -1;
	}
	ASSERT_DEBUG(TST_START, (prio >= 1) && (prio <= MAXPRIO));

	/*********************************************/

	/*
	 * Corps de la fonction
	 */
	
	// Obtention pid.
	if ((new_pid = obtain_pid()) == -1) {
		// échec obtention pid
		PRINT_DEBUG(TST_START, 
				3, 
				"<<<<<< %i exit start, NO_PID error\n", 
				cur_pid);
		return -1;
	}
	ASSERT_DEBUG(TST_START, new_pid != -1);

	/*********************************************/

	// Obtention memoire pour le nom + copie nom.
	process_table[new_pid].name = 
		mem_alloc(strnlen(name, MAX_SIZE_PROCESS_NAME)+1);
	if (process_table[new_pid].name == 0) {
		// échec allocation mémoire nom
		PRINT_DEBUG(TST_START, 
				3, 
				"<<<<<< %i exit start, PROC_NAME error\n", 
				cur_pid);
		return -1;
	}
	ASSERT_DEBUG(TST_START, 
			process_table[new_pid].name != 0);
	strncpy(process_table[new_pid].name, 
			name, 
			strnlen(name, MAX_SIZE_PROCESS_NAME) + 1);

	/*********************************************/

	unsigned long stack_size;
	void *stack;

	// Allocation & Initialisation de la pile user.
	stack_size = ssize + 2 * sizeof(int);
	stack = user_stack_alloc(stack_size);
	if (stack == 0) {
		// echec allocation pile user.
		mem_free(process_table[new_pid].name, strnlen(process_table[new_pid].name, MAX_SIZE_PROCESS_NAME) + 1);
		PRINT_DEBUG(TST_START, 3, "<<<<<< %i exit start, STACK error\n", cur_pid);
		return -1;
	}
	ASSERT_DEBUG(TST_START, stack != 0);

	process_table[new_pid].user_stack_size = stack_size;
	process_table[new_pid].user_stack = stack;

	*((int *)(stack + stack_size - 2 * sizeof(int))) = (int)PROCESS_RETURN_ADR;
	*((int *)(stack + stack_size - sizeof(int))) = (int)arg;

	// Allocation & Initialisation de la pile kernel.
	stack_size = KERNEL_STACK_SIZE;
	stack = mem_alloc(stack_size);
	if (stack == 0) {
		// echec allocation pile kernel.
		mem_free(process_table[new_pid].name, strnlen(process_table[new_pid].name, MAX_SIZE_PROCESS_NAME) + 1);
		user_stack_free(process_table[new_pid].user_stack, process_table[new_pid].user_stack_size);
		PRINT_DEBUG(TST_START, 3, "<<<<<< %i exit start, STACK error\n", cur_pid);
		return -1;
	}
	ASSERT_DEBUG(TST_START, stack != 0);

	process_table[new_pid].kernel_stack = stack;

	*((int *)(stack + stack_size - 6 * sizeof(int))) = (int)(&goto_user_mode);
	*((int *)(stack + stack_size - 5 * sizeof(int))) = (int)(ptfunc);
	*((int *)(stack + stack_size - 4 * sizeof(int))) = USER_CS;
	*((int *)(stack + stack_size - 3 * sizeof(int))) = 0x202;
	*((int *)(stack + stack_size - 2 * sizeof(int))) =
		(int)(process_table[new_pid].user_stack + process_table[new_pid].user_stack_size - 2 * sizeof(int));
	*((int *)(stack + stack_size - sizeof(int))) = USER_DS;

	process_table[new_pid].zone_reg[0] = (int)(stack + stack_size - 6 * sizeof(int)); // MaJ %esp

	// Champ de chainage pour la garbage_collection_list.
	process_table[new_pid].garbage_collection.next = 0;
	process_table[new_pid].garbage_collection.prev = 0;

	/*********************************************/

	// Initialisation des champs
	process_table[new_pid].pid = new_pid;
	process_table[new_pid].alarm_time = 0;
	process_table[new_pid].prio = prio;
	process_table[new_pid].retvalue = 0;

	/*********************************************/

	// Initialisation des champs pour l'ordonnancement.
	process_table[new_pid].state = actionable;
	process_table[new_pid].chaining.next = 0;
	process_table[new_pid].chaining.prev = 0;
	queue_add(&process_table[new_pid],&queue_table[actionable],process,chaining,prio);

	/*********************************************/

	// Initialisation des champs pour la gestion de la filiation.
	process_table[new_pid].children_chaining.next = 0;
	process_table[new_pid].children_chaining.prev = 0;
	INIT_LIST_HEAD(&(process_table[new_pid].children));

	process_table[new_pid].child_waited = 0;

	process_table[new_pid].father = cur_pid;
	queue_add(&process_table[new_pid],&(process_table[cur_pid].children),process,children_chaining,prio);

	/*********************************************/

	// Appel à scheduler si processus créé plus prioritaire
	// cur_pid == 0 indique que le processus est créé dans kernel (pas d'ordonnancement)
	if (cur_pid != 0 && process_table[cur_pid].prio < prio) {
		scheduler();
	}

	/*********************************************/

	PRINT_DEBUG(TST_START, 3, "<<<<<< %i exit start, pid %i given\n", cur_pid, new_pid);
	return new_pid;
}

/* 
 * Attente temporisee
 */  
void wait_clock(unsigned long clock){
  if(clock>current_clock()) {
	process_table[cur_pid].alarm_time = clock;

	queue_add(&process_table[cur_pid], &queue_table[asleep], 
			process, chaining, alarm_time);
	process_table[cur_pid].state=asleep;
	scheduler();
  }
}


/* 
 * Endormissement processus
 */
void sleep(unsigned long time) {
	wait_clock(time*CLOCKFREQ);
}

/******************************************************************************/
/*                             Gestion filiation                              */
/******************************************************************************/

/*
 * Signalement au pere la terminaison du fils
 */
int report_father(int pid){
	int father = process_table[pid].father;
	if(process_table[father].child_waited==pid 
			|| process_table[father].child_waited==-1) {
		// Si le processus attendu est le processus courant, le pere est reveille
		process_table[father].child_waited=pid;
		queue_del(find_pid(&queue_table[waiting_child],father), chaining);
		queue_add(&process_table[father], &queue_table[actionable], 
				process, chaining, pid);
		process_table[father].state=actionable;
		return pid;
	}
	return -1;
}


/*
 * Tue les zombies fils du père à sa mort
 */
void check_zombies_children(int father) {
	process* cur;
	int pid;
	int zombies = 1;
	while(zombies!=0) {
		zombies=0;
		queue_for_each(cur, &process_table[father].children, process, children_chaining){
			if (cur->state==zombie) {
				zombies++;
				pid=cur->pid;
				queue_del(cur, children_chaining);
				queue_del(find_pid(&queue_table[zombie],pid),chaining); // TODO change find_pid par cur
				free_pid(pid);
				break;
			}
		}	
	}	
}

/* 
 * Informe les enfants de la mort de leur pere
 */
void funeral(int father) {
	process* cur;
	queue_for_each(cur, &process_table[father].children, process, children_chaining){
		(*cur).father=0;
	}
}

/* 
 * Attente de la terminaison d'un (ou de l'un des) fils
 */
int waitpid(int pid, int *retvalp){
	/* Vérification des arguments */
	/******************************/
	
	// validité pid globale
	if (pid != -1 && (pid <= 0 || pid >= NB_PROCESS)) {
		return -1;
	}

	// pid est un fils du processus appelant
	if (queue_empty(&process_table[cur_pid].children)) {
		return -1;
	}
	if (pid >= 0 && !find_pid_children(&process_table[cur_pid].children,pid)) {
		return -1;
	}

	/* Corps de fonction */
	/*********************/

	process_table[cur_pid].child_waited = pid;

	// Le processus appelant doit-il attendre ?
	if (pid == -1) {
		process *cur;
		// recherche d'un fils zombie
		queue_for_each_prev(cur, &process_table[cur_pid].children, process, children_chaining) {
			if (cur->state == zombie) {
				process_table[cur_pid].child_waited = cur->pid;
				break;
			}
		}
		// si il n'y a pas de fils zombie, il faut attendre
		if (process_table[cur_pid].child_waited == -1) {
			queue_add(&process_table[cur_pid], &queue_table[waiting_child], 
					process, chaining, pid);
			process_table[cur_pid].state = waiting_child;
			scheduler();
		}
	} else {
		// si le fils n'est pas zombie il faut attendre
		if (process_table[pid].state != zombie) {
			queue_add(&process_table[cur_pid], &queue_table[waiting_child], 
					process, chaining, pid);
			process_table[cur_pid].state = waiting_child;
			scheduler();
		}
	}

	// Arrivé ici, le fils attendu est mort : libération & récuperation de retval
	// si pid=-1, child_waited=pid du fils qui a repondu
	if(retvalp != 0){
		*retvalp = process_table[process_table[cur_pid].child_waited].retvalue;
	}
	process* cur = find_pid(&queue_table[zombie],
			process_table[cur_pid].child_waited);
	queue_del(cur, chaining);
	queue_del(cur, children_chaining);
	free_pid(process_table[cur_pid].child_waited);
	int dead_child = process_table[cur_pid].child_waited;
	process_table[cur_pid].child_waited = 0;
	return dead_child;
}

/* 
 * Demon d'elimination des zombies qui n'ont pas de père qui les attend
 * TODO -- code mort ????
 */
void check_zombies(void) {
	process* cur;
	int pid;
	int zombies = 0;
	int time = (int)current_clock();

	queue_for_each(cur, &queue_table[zombie], process, chaining){
		zombies++;
	}


	zombies=1;
	while(zombies!=0) {
		zombies=0;
		queue_for_each(cur, &queue_table[zombie], process, chaining){
			if (find_pid(&queue_table[waiting_child],cur->father)==0 && (cur->alarm_time+5*CLOCKFREQ)<time) {
				// Ne supprime que les processus zombies depuis plus de 5 secondes
				zombies++;
				pid = cur->pid;
				queue_del(find_pid_children(&process_table[cur->father].children,pid),children_chaining);
				queue_del(cur, chaining);
				free_pid(pid);
				break;
			}
		}
	}

	zombies = 0;
	queue_for_each(cur, &queue_table[zombie], process, chaining){
		zombies++;
	}


}

/******************************************************************************/
/*                           Terminaison processus                            */
/*                         ~ Récupération de la mémoire ~                     */
/******************************************************************************/

/*
 * Ajout d'un processus à la garbage_collection_list.
 */
static void add_garbage_collection(int pid) {
	queue_add(&process_table[pid], &garbage_collection_list, process, garbage_collection, pid);
}

/*
 * Indique si il faut récupérer de la mémoire.
 */
static int garbage_collection_needed(void) {
	return !queue_empty(&garbage_collection_list);
}

static void garbage_collect(void) {

	process *dead_process;

	while (garbage_collection_needed()) {
		dead_process = queue_out(&garbage_collection_list, process, garbage_collection);

		PRINT_DEBUG(TST_MEMORY, 3, "------ liberation de la memoire de %i\n", dead_process->pid);
		mem_free(dead_process->kernel_stack, KERNEL_STACK_SIZE);
		user_stack_free(dead_process->user_stack, dead_process->user_stack_size);

		if (dead_process->father == 0) {
			queue_del(dead_process, chaining);
			queue_del(dead_process, children_chaining);
			free_pid(dead_process->pid);
		}
	}
}
/*
 * Récupération de la mémoire d'un processus qui vient de mourir.
 */
static void garbage_collector(void) {

	for (;;) {
		garbage_collect();
		scheduler();
	}
}

/*
 * Initialisation du processus de récuperation de la mémoire.
 */
static void init_garbage_collector(void) {
	process_table[PID_GARBAGE].name = (char *)mem_alloc(18);
	strcpy(process_table[PID_GARBAGE].name, "garbage collector");

	process_table[PID_GARBAGE].pid = PID_GARBAGE;

	process_table[PID_GARBAGE].kernel_stack = mem_alloc(KERNEL_STACK_SIZE);

	*((int *)(process_table[PID_GARBAGE].kernel_stack + KERNEL_STACK_SIZE - sizeof(int))) = (int)(&garbage_collector);
	process_table[PID_GARBAGE].zone_reg[0] = (int)(process_table[PID_GARBAGE].kernel_stack + KERNEL_STACK_SIZE - sizeof(int));
}

/******************************************************************************/
/*                           Terminaison processus                            */
/*                         ~ Fonctions exit & kill ~                          */
/******************************************************************************/

/*
 * Terminaison d'un processus de sa propre initiative
 */
void exit(int retval) {
  //PRINT_DEBUG(TST_EXIT, 3, "------ %i exit, retval %i\n", cur_pid, retval);
	// MaJ retval
	process_table[cur_pid].retvalue = retval;

	// suppression activable

	process_table[cur_pid].alarm_time = (int)current_clock();

	// devient zombie
	queue_add(&process_table[cur_pid], &queue_table[zombie], 
			process, chaining, alarm_time);
	process_table[cur_pid].state=zombie;
	report_father(cur_pid);
	check_zombies_children(cur_pid);
	funeral(cur_pid);

	add_garbage_collection(cur_pid);

	if (process_table[cur_pid].state==sem_blocked) {
	  int semval = process_table[cur_pid].sem_blocked;
	  process_table[cur_pid].sem_blocked = -1;
	  signal(semval);
	}

	scheduler();

	// on ne doit pas revenir de exit
	while(1);
}

/*
 * Terminaison externe
 */
int kill(int pid) {
	PRINT_DEBUG(TST_KILL, 3, ">>>>>> %i trying to kill %i\n", cur_pid, pid);
	if (pid <= 0 || pid >= NB_PROCESS)  {	// idle ne peut pas être tué (donc <= 0)
		PRINT_DEBUG(TST_KILL, 3, "<<<<<< %i kill BAD_PID(%i) error\n", cur_pid, pid);
		return -1;
	}
	ASSERT_DEBUG(TST_KILL, pid > 0 && pid < NB_PROCESS);	

	if (pid == cur_pid) {
		PRINT_DEBUG(TST_KILL, 3, "<<<<<< %i kill myself !?, did you mean exit(%i) ??\n", cur_pid, KILL_RETVAL);
		exit(KILL_RETVAL);
	}

	process_table[pid].retvalue = KILL_RETVAL;

	process* cur;
	int semval;
	switch(process_table[pid].state) {
		case actionable :
			cur = find_pid(&queue_table[actionable],pid);
			queue_del(cur, chaining);
			break;
		case asleep :
			cur = find_pid(&queue_table[asleep],pid);
			queue_del(cur, chaining);
			break;
		case sem_blocked :
		        semval = process_table[pid].sem_blocked;
			process_table[pid].sem_blocked = -1;
			signal_death(semval);
			cur = find_pid(&queue_table[sem_blocked],pid);
			queue_del(cur, chaining);
			break;
		case io_blocked :
			cur = find_pid(&queue_table[io_blocked],pid);
			queue_del(cur, chaining);
			break;
		case waiting_child :
			cur = find_pid(&queue_table[waiting_child],pid);
			queue_del(cur, chaining);
			break;
		default :
			PRINT_DEBUG(TST_KILL, 3, "<<<<<< %i kill NOT_FOUND_PID(%i) error\n", cur_pid, pid);
			return -1;
	}
	process_table[pid].alarm_time = (int)current_clock();

	// MaJ dans les structures de l'ordonnanceur

	queue_add(&process_table[pid], &queue_table[zombie], 
			process, chaining, alarm_time);
	process_table[pid].state=zombie;
	report_father(pid);
	check_zombies_children(pid);
	funeral(pid);

	add_garbage_collection(pid);
	garbage_collect();

	PRINT_DEBUG(TST_KILL, 3, "<<<<<< %i killed %i\n", cur_pid, pid);
	return 0;
}


/******************************************************************************/
/*                            Gestion semaphores                              */
/******************************************************************************/

/*
 * DEBUG
 */
void print_list_sem(void) 
{
  process* cur;
  queue_for_each_prev(cur, &queue_table[sem_blocked], process, chaining)
    {
      printf("processus %s de prio %d ;",cur->name,cur->prio);
    }
  printf("\n");
}


/*
 * Recherche par semaphore d'un processus
 */
static process* find_by_sem(int sem) 
{
  process* cur;
  queue_for_each_prev(cur, &queue_table[sem_blocked], process, chaining)
    {
      if (cur->sem_blocked==sem) {
	return cur;
      }
    }
  return 0;
}

/* 
 * Retourne une indication sur le mecanisme qui a reveille le processus :
 * 1 )  0 si le reveil est consecutif à signal
 * 2 ) -3 si le reveil est consecutif à sdelete
 * 3 ) -4 si le reveil est consecutif à sreset
 */
int get_awakener(){
	return process_table[cur_pid].awakener;
}

/* 
 * Bloque un processus sur le semaphore "sem"
 * Retourne son pid en cas de succes
 */
void block_sem(int sem) {
  process_table[cur_pid].sem_blocked=sem;
  process_table[cur_pid].state=sem_blocked;
  queue_add(&process_table[cur_pid], 
	    &queue_table[sem_blocked], 
	    process, 
	    chaining, 
	    prio);
  scheduler();
}

/* 
 * Trouve le plus prioritaire des processus bloques par le semaphore 
 * en parametre, le retire de la liste des processus bloques
 * et le place dans la liste des activables.
 */
int unblock_sem(int sem) {
  int pid=-1;
  process* cur=find_by_sem(sem);
  if(cur!=0){
    pid=cur->pid;
    queue_del(cur, chaining);
    process_table[cur->pid].state=actionable;
    process_table[cur->pid].awakener=0;
    cur->sem_blocked=-1;
    queue_add(cur, 
	      &queue_table[actionable], 
	      process, 
	      chaining, 
	      prio);
  }
  return pid;
}


/* 
 * Trouve les n plus prioritaire des processus bloques par le semaphore 
 * en parametre, les retire de la liste des processus bloques
 * et les place dans la liste des activables.
 */
void unblock_n_sems(int sem, short int n) {
  if(n<=0){
    return;
  }
  short int count = 0;
  process* cur=find_by_sem(sem);
  
  while((cur!=0)&&(count<n)){
    count++;
    queue_del(cur, chaining);
    process_table[cur->pid].state=actionable;
    process_table[cur->pid].awakener=0;
    process_table[cur->pid].sem_blocked=-1;
    queue_add(cur, 
	      &queue_table[actionable], 
	      process, 
	      chaining, 
	      prio);
    cur=find_by_sem(sem);
  }
  cur= queue_top(&queue_table[actionable], process, chaining);
  if (cur->prio > process_table[cur_pid].prio){
    scheduler();
  }
}





/* 
 * Libere tous les processus bloques sur le semaphore "sem"
 */
void unblock_all_sem(int sem,int wake) {
  process* cur=find_by_sem(sem);
  while(cur!=0){
    queue_del(cur, chaining);
    process_table[cur->pid].state=actionable;
    process_table[cur->pid].awakener=wake;
    cur->sem_blocked=-1;
    queue_add(cur, 
	      &queue_table[actionable], 
	      process, 
	      chaining, 
	      prio);
    cur=find_by_sem(sem);
  }
  cur= queue_top(&queue_table[actionable], process, chaining);
  if (cur->prio > process_table[cur_pid].prio){
    scheduler();
  }
}

/******************************************************************************/
/*                              Entrees/Sorties                               */
/******************************************************************************/

/*
 * wait sur entree/sortie
 */
int wait_on_io(){
  //cli();

  // Valeur negative
  if (read_mutex<0){
    process_table[cur_pid].state=io_blocked;
    queue_add(&process_table[cur_pid], 
	      &queue_table[io_blocked], 
	      process, 
	      chaining, 
	      prio);
    //sti();
    scheduler();
    return  cur_pid;
  }
  //sti();
  return cur_pid;
}

/*
 * signal sur entree/sortie
 */
static int signal_cons_read_io(void){
  //cli();
  if(queue_empty(&queue_table[io_blocked])){
    //sti();
    return -1;
  }
  read_mutex=read_mutex+1;
  process* cur=queue_out(&queue_table[io_blocked],process,chaining);
  process_table[cur->pid].state=actionable;
  queue_add(cur, 
	    &queue_table[actionable], 
	    process, 
	    chaining, 
	    prio);
  //sti();
  scheduler();
  return cur->pid;
}

/*
 * Activation/desactivation de l'echo sur l'ecran
 */ 
void cons_echo(int on){
  set_echo(on);
}

/*
 * Lecture sur interruptions clavier
 */ 
unsigned long cons_read(char *string, unsigned long length){
  /* Requiert : string deja alloue */
  //printf("processus %s se bloque sur cons_read\n",process_table[cur_pid].name);
  // Un seul lecteur possible
  wait_on_io();

  // Taille nulle 
  if(length==0){    
    // Deblocage processus
    signal_cons_read_io();
    return 0;
  }

  // Resultat
  unsigned long res =0;

  
  /* Cas 1) : Tampon clavier non vide */ 
  if(!buff_empty()){
    
    // Recopie a partir du tampon clavier
    res=copy_from_buffer(string,length);
    printf("\n");

    // Deblocage processus
    signal_cons_read_io();   
    // Nombre de caracteres lus effectivement
    return res;
  }
  /* Cas 2) : Tampon clavier vide => blocage processus */

  // Blocage sur entree sortie
  queue_add(&process_table[cur_pid], &queue_table[io_blocked], 
	    process, chaining, prio);
  process_table[cur_pid].state=io_blocked;

  // Signalement debut lecture 
  enable_reading(cur_pid);

  // Appel ordonnanceur
  scheduler();
  //printf("processus %s fait de la recopie\n",process_table[cur_pid].name);
  // Recuperation resultat 
  res=copy_from_buffer(string,length);
  printf("\n");

  // Liberation section critique
  signal_cons_read_io();
  // Nombre de caracteres lus effectivement
  return res;
}


/*
 * Signalement fin d'interruption
 */
void signal_end_io(int pid){
  //cli();
  read_mutex=read_mutex+1;
  process* cur = find_pid(&queue_table[io_blocked],pid);
  queue_del(cur, chaining);
  cur->state=actionable;
  queue_add(cur, &queue_table[actionable], 
	    process, chaining, prio);
  
  //sti();
  scheduler();  
}


/******************************************************************************/
/*                               Ordonnancement                               */
/******************************************************************************/

/*
 * Ordonnanceur
 */

static void init_scheduler(void) {
	start(&idle, 0, 1, "idle", 0); // premier processus à démarrer (pid 0)
	process_table[0].prio = 0;
	queue_del(&process_table[0], chaining);
}

void scheduler(void) {
	int old_pid = cur_pid; // sauvegarde du pid en cours  
	int time = current_clock();
	process* cour;
	// Vérifie que des processus ne sont pas prêts à être réveillés
	while(!queue_empty(&queue_table[asleep]) 
			&& (*queue_bottom(&queue_table[asleep], 
					process, 
					chaining)).alarm_time
			< time){
		cour = queue_out_bot(&queue_table[asleep], 
				     process, 
				     chaining);
		(*cour).alarm_time=0;
		queue_add(cour, 
				&queue_table[actionable], 
				process, 
				chaining, 
				prio);
		(*cour).state=actionable;
	}

	// MaJ des processus activables
	if (process_table[old_pid].state == actionable 
	    && old_pid != PID_GARBAGE) {
		queue_add(&process_table[old_pid], 
			  &queue_table[actionable], 
			  process, 
			  chaining, 
			  prio);
	}

	// Changement de contexte
	if (garbage_collection_needed()) {
		//  On ordonnance le "garbage collector" en priorite 
		// si de la mémoire est à libérer
		cur_pid = PID_GARBAGE;
	} else {
		// Sinon on ordonnance classiquement
		process *next = queue_out(&queue_table[actionable], 
				process, 
				chaining);
		cur_pid = next->pid;
	}
	
	tss.esp0 = (int)(process_table[cur_pid].kernel_stack+KERNEL_STACK_SIZE); // TODO -- ajouter champ esp0 dans structure processus pour eviter recalcul
	ctx_sw(&(process_table[old_pid].zone_reg[0]),
			&(process_table[cur_pid].zone_reg[0]));
}

/*******************************************************************************/
/*                              Initialisation                                 */
/*******************************************************************************/

/*
 * Initialisation de l'ordonnanceur : mise en place des processus kernels.
 */
void init_process_handling(void) {

	init_free_list();
	init_scheduler();
	init_garbage_collector();
}



/*******************************************************************************/
/*                                    SHELL                                    */
/*******************************************************************************/

/*
 * Lecture sur interruptions clavier (cas du shell)
 */ 
unsigned long shell_cons_read(char *string, unsigned long length){
  /* Requiert : string deja alloue */

  // Un seul lecteur possible
  wait_on_io();
  // Taille nulle 
  if(length==0){     
    // Deblocage processus
    signal_cons_read_io();
    return 0;
  }

  // Resultat
  unsigned long res =0;

  
  /* Cas 1) : Tampon clavier non vide */ 
  if(!buff_empty()){
    
    // Recopie a partir du tampon clavier
    res=copy_from_buffer(string,length);
    set_col_min(0);
    printf("\n");

    // Deblocage processus
    signal_cons_read_io();
   
    // Nombre de caracteres lus effectivement
    return res;
  }

  /* Cas 2) : Tampon clavier vide => blocage processus */
  //process* cur = find_pid(&queue_table[actionable],cur_pid);
  queue_add(&process_table[cur_pid], &queue_table[io_blocked], 
	    process, chaining, prio);
  process_table[cur_pid].state=io_blocked;

  // Signalement debut lecture 
  enable_reading(cur_pid);
  scheduler();

  // Recuperation resultat et liberation section critique
  res=copy_from_buffer(string,length);
  set_col_min(0);
  printf("\n");

  // Deblocage processus
  signal_cons_read_io();

  // Nombre de caracteres lus effectivement
  return res;
}


/*
 * Liste des processus
 */
static void list(link * l, char * state){
  process * cur;
  queue_for_each(cur,l, process, chaining){
    printf("%d              %s              %s \n",cur->pid,cur->name,state);
  }
}

void list_ps(void){
  set_col_min(0);
  printf("pid             nom             etat\n");
  printf("---------------------------------------------\n");
  list(&queue_table[actionable], "activable");
  list(&queue_table[asleep], "endormi");
  list(&queue_table[sem_blocked], "bloque sur semaphore");
  list(&queue_table[io_blocked], "bloque sur entree/sortie");
  list(&queue_table[waiting_child], "en attente d'un fils");
  list(&queue_table[zombie], "zombie");
  printf("\n");
}

/*
 * Prompt du shell
 */
void print_prompt(void){
  update_buffer();
  set_col_min(0);
  printf(PROMPT);
  set_col_min(PROMPT_SIZE);
}  

/*
 * Effacement ecran clavier (cas du shell)
 */ 
void clear(void){
  set_col_min(0);
  handle_char(12);
}


