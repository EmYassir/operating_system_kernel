#ifndef __SYSLIB_H__
#define __SYSLIB_H__

/*  Lors de l'appel à syscall, call=n° de la procédure dans l'ordre 
 *   de ce fichier à partir de zéro 
 */

#include "audio_format.h"

int chprio(int pid, int newprio); 

void clock_settings(unsigned long *quartz, unsigned long *ticks);

void cons_echo(int on);

unsigned long cons_read(char *string, unsigned long length);

int cons_write(const char *str, long size);

unsigned long current_clock();

void exit(int retval);

int getpid(void);

int getprio(int pid);

int kill(int pid);

int scount(int sem);

int screate(short int count);

int sdelete(int sem);

int sreset(int sem, short int count);

int signal(int sem);

int signaln(int sem, short int count);

int start(int (*ptfunc)(void *), unsigned long ssize, int prio, const char *name, void *arg);

int try_wait(int sem);

int wait(int sem);

void wait_clock(unsigned long clock);

int waitpid(int pid, int *retvalp);

void play_music(track *music);

#endif /* __SYSLIB_H__ */
