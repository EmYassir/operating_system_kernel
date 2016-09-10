#include "syslib.h"
#include "audio_format.h"

extern int syscall(int call, int par1, int par2, int par3, int par4, int par5);

int chprio(int pid, int newprio){
  // call = 1
  return syscall(1, pid, newprio, 0, 0, 0);
}
void clock_settings(unsigned long *quartz, unsigned long *ticks){
  // call = 2
  syscall(2, (int)quartz, (int)ticks, 0, 0, 0);
}
void cons_echo(int on){
  // call = 3
  syscall(3, (int)on, 0, 0, 0, 0);
}

unsigned long cons_read(char *string, unsigned long length){
  // call = 4
  return (unsigned long)(syscall(4, (int)string, (int)length, 0, 0, 0));
}

int cons_write(const char *str, long size) {
  // call = 5
  syscall(5, (int)str, (int)size, 0, 0, 0);
  return 0;
}

unsigned long current_clock(){
 // call = 6 
  return (unsigned long)(syscall(6, 0, 0, 0, 0, 0));
}

void exit(int retval) {
  // call = 7
  syscall(7, retval, 0, 0, 0, 0);
  while(1);

}

int getpid(void) {
  // call = 8
  return syscall(8, 0, 0, 0, 0, 0);
}


int getprio(int pid){
  // call = 9
  return syscall(9, pid, 0, 0, 0, 0);
}

int kill(int pid){
  // call = 10
  return syscall(10, pid, 0, 0, 0, 0);
}

int scount(int sem){
  // call = 11
  return syscall(11, sem, 0, 0, 0, 0);
}

int screate(short int count){
  // call = 12
  return syscall(12, count, 0, 0, 0, 0);
}
int sdelete(int sem){
  // call = 13
  return syscall(13, sem, 0, 0, 0, 0);
}
int sreset(int sem,short int count){
  // call = 14
  return syscall(14, sem, count, 0, 0, 0);
}
int signal(int sem){
  // call = 15
  return syscall(15, sem, 0, 0, 0, 0);
}

int signaln(int sem, short int count){
  // call = 16
  return syscall(16, sem, count, 0, 0, 0);
}


int start(int (*ptfunc)(void *), unsigned long ssize, int prio, const char *name, void *arg) {
  // call = 17
  return syscall(17, (int)ptfunc, (int)ssize, prio, (int)name, (int)arg);
}

void wait_clock(unsigned long clock) {
  // call = 20
  syscall(20, (int)clock, 0, 0, 0, 0);
}

int waitpid(int pid, int *retvalp) {
  // call = 21
  return syscall(21, pid, (int)retvalp, 0, 0, 0);
}

int wait(int sem){
  // call = 22
  return syscall(22, sem, 0, 0, 0, 0);
}

int try_wait(int sem){
  // call = 23
  return syscall(23, sem, 0, 0, 0, 0);
}

void play_music(track *music)
{
	syscall(36, (int)music, 0, 0, 0, 0);
}
