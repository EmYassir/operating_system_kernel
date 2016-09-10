#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__

#include "sem.h"
#include "common.h"
#include "queue.h"
#include "arch/cc.h"
#include "string.h"

typedef s32_t sys_thread_t;
typedef s32_t sys_sem_t;
typedef link* sys_mbox_t;

#define SYS_MBOX_NULL 0
#define SYS_SEM_NULL 0

extern link mboxes[NB_MBOXES];

sys_thread_t sys_thread_new(void (* thread)(void *arg), void *arg, int prio);

void sys_init(void);

sys_sem_t sys_sem_new(u8_t count);

void sys_sem_free(sys_sem_t sem);

void sys_sem_signal(sys_sem_t sem);

// implem fausse mais esperons fonctionnelle
u32_t sys_arch_sem_wait(sys_sem_t sem, u32_t timeout);


sys_mbox_t sys_mbox_new(void);
// Creates an empty mailbox.

void sys_mbox_free(sys_mbox_t mbox);
// Deallocates a mailbox. If there are messages still present in the
// mailbox when the mailbox is deallocated, it is an indication of a
// programming error in lwIP and the developer should be notified.

void sys_mbox_post(sys_mbox_t mbox, void *msg);
// Posts the "msg" to the mailbox.

u32_t sys_arch_mbox_fetch(sys_mbox_t mbox, void **msg, u32_t timeout);
// Blocks the thread until a message arrives in the mailbox, but does
// not block the thread longer than "timeout" milliseconds (similar to
// the sys_arch_sem_wait() function). The "msg" argument is a result
// parameter that is set by the function (i.e., by doing "*msg =
// ptr"). The "msg" parameter maybe NULL to indicate that the message
// should be dropped.
// The return values are the same as for the sys_arch_sem_wait() function:
// Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
// timeout.

struct sys_timeouts *sys_arch_timeouts(void);

#endif /* __SYS_ARCH_H__ */


