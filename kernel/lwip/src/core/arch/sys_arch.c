#include "arch/sys_arch.h"
#include "timer.h"
#include "process_handler.h"
#include "lwip/mem.h"
#include "lwip/sys.h"
#include "stdio.h"

// Tableau des boites aux lettres
link mboxes[NB_MBOXES]; 
// Tableau des listes de threads attendant chaque boite aux lettres
link postmen[NB_MBOXES];

int net_awakening_daemon_pid;
int net_awakening_daemon_sem = -1;
int sound_asleep_daemon = 0;
// 0 si le demon attend un message, 1 s'il n'y a plus de thread en attente

// Default timeouts list for lwIP
struct sys_timeouts* lwip_system_timeouts = 0; 
int IMPLEMENTE = 0;

typedef struct _message {
  void* msg;
  unsigned long time;
  link chaining;
}message;

typedef struct _postman {
  int pid;
  unsigned long time;
  link chaining;
}postman;

void net_wake_up(int box) {

  postman* waiting_thread = queue_out_bot(&(postmen[box]), postman, chaining);
  if (waiting_thread!=0) {
    signal_end_io(waiting_thread->pid);
    lmem_free((void*)waiting_thread);
  }

}

void net_sleep(sys_mbox_t mbox, u32_t timeout) {

  postman* waiting_thread = (postman*)mem_malloc(sizeof(postman));
  waiting_thread->pid = getpid();
  waiting_thread->time = current_clock() + timeout*CLOCKFREQ/1000;
  wait_on_io();

}

// Processus reveillant les threads sur boites aux lettres
// endormis et attend le suivant
void net_awakening_daemon () {

  unsigned long next_time = 4000000000;
  int next_mbox = NB_MBOXES+1;
  postman* current;

  net_awakening_daemon_pid = getpid();
  net_awakening_daemon_sem = screate(0);
  while(1) {
    if (next_mbox!=NB_MBOXES+1) {
      current = queue_bottom(&postmen[next_mbox], postman, chaining);
      if (current != 0) {
        if (current->time==next_time) {
          net_wake_up(next_mbox);
        } // si non, thread deja reveille
      }
    }
    for (int i=0; i<NB_MBOXES; i++) {
      current = queue_bottom(&postmen[i], postman, chaining);
      if (current != 0) {
        if (current->time<next_time) {
          next_mbox = i;
          next_time = current->time;
        }
      }
    }
    wait(net_awakening_daemon_sem);
  }
}

sys_mbox_t sys_mbox_new(void) {

  for (int i=0; i<NB_MBOXES; i++) {
    if(mboxes[i].prev!=0 && mboxes[i].next!=0) {
      INIT_LIST_HEAD(&mboxes[i]);
      INIT_LIST_HEAD(&postmen[i]);
      return &mboxes[i];
    }
  }
  return 0;
}

void sys_mbox_free(sys_mbox_t mbox) {

  mbox->prev = 0;
  mbox->next = 0;

}

void sys_mbox_post(sys_mbox_t mbox, void *msg) {

  message* new_msg = (message*)mem_malloc(sizeof(message));
  new_msg->msg = msg;
  new_msg->time = current_clock();
  queue_add(new_msg, mbox, message, chaining, time);
  net_wake_up((mbox-&mboxes[0])/sizeof(link));

}

u32_t sys_arch_mbox_fetch(sys_mbox_t mbox, void **msg, u32_t timeout) {

  u32_t beginning = current_clock()*CLOCKFREQ/1000;
  if (sound_asleep_daemon==0) {
    unblock_sem(net_awakening_daemon_sem);
  }
  net_sleep(mbox, timeout);
  message* new_msg = queue_out_bot(mbox, message, chaining);
  if (new_msg!=0) {
    msg = &(new_msg->msg);
    lmem_free((void*)new_msg);
    return (current_clock() - beginning)*1000/CLOCKFREQ;
  } 
  return SYS_ARCH_TIMEOUT;
}

sys_sem_t sys_sem_new(u8_t count) {
  return screate(count);
}

void sys_sem_free(sys_sem_t sem) {
  sdelete(sem);
}

void sys_sem_signal(sys_sem_t sem) {
  signal(sem);
}

u32_t sys_arch_sem_wait(sys_sem_t sem, u32_t timeout) {
  unsigned long time = current_clock();
  if(timeout!=0) {
    wait(sem);
  } else {
    wait_clock(timeout*CLOCKFREQ/1000);
  }
  return (current_clock()-time);
}

sys_thread_t sys_thread_new(void (* thread)(void *arg), void *arg, int prio) {
  return start((int (*)(void *))thread, 0x20000, prio, "lwIP_thread", arg);
}

void sys_init(void) {

}

struct sys_timeouts *sys_arch_timeouts(void) {
  assert(IMPLEMENTE);
  return lwip_system_timeouts;
}
