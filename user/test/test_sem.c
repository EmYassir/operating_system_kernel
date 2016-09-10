/*
 * test_sem.c
 *
 * Copyright (C) 2012 by group psys-7
 *
 * Base de tests pour les sémaphores.
 */

#include "stdio.h"
#include "string.h"
#include "../../kernel/common.h"
#include "test_all.h"
#include "test.h"
#include "utils.h"
#include "syslib.h"
#include "../../kernel/debugger.h"

/******************************************************************************/
/*                              Fonctions utiles                              */
/******************************************************************************/

/*
 * Generation nombres pseudo-aleatoires
 */
unsigned int lfsr113_Bits(void)
{
  static unsigned int z1 = 12345, z2 = 12345, z3 = 12345, z4 = 12345;
  unsigned int b;
  b  = ((z1 << 6) ^ z1) >> 13;
  z1 = ((z1 & 4294967294U) << 18) ^ b;
  b  = ((z2 << 2) ^ z2) >> 27; 
  z2 = ((z2 & 4294967288U) << 2) ^ b;
  b  = ((z3 << 13) ^ z3) >> 21;
  z3 = ((z3 & 4294967280U) << 7) ^ b;
  b  = ((z4 << 3) ^ z4) >> 12;
  z4 = ((z4 & 4294967168U) << 13) ^ b;
  return (z1 ^ z2 ^ z3 ^ z4);
}

int rand_a_b(int a, int b){
  return (lfsr113_Bits()%(b-a) +a);
}


int my_rand(){
  return rand_a_b(1, 10);
}

/******************************************************************************/
/*                               TESTS SEMAPHORES                             */
/******************************************************************************/

/****************************
 * Tests semaphore simples  *
 ****************************/

// Test creation destruction
int test_sem_cd() {
  PRINT_DEBUG(TST_SEM, 2, "************[test_sem_cd]*************\n");
  int s = screate(4);
  ASSERT_DEBUG(TST_SEM, s==0);
  int res =sdelete(s);
  ASSERT_DEBUG(TST_SEM, res==0);
  return s;
}


// Test destruction invalide
int test_sem_id() { 
  PRINT_DEBUG(TST_SEM, 2, "************[test_sem_id]*************\n");
  int test = sdelete(4);
  ASSERT_DEBUG(TST_SEM, test==-1);
  return test;
}


// Tests creation invalides:

// 1 ) creation avec count negatif
int test_sem_ic_1() {
  PRINT_DEBUG(TST_SEM, 2, "***********[test_sem_ic_1]************\n");
  int s=screate(-1);
  ASSERT_DEBUG(TST_SEM, s==-1);
  return s;
}

// 2 ) creation a l'infini
int test_sem_ic_2() {
  PRINT_DEBUG(TST_SEM, 2, "***********[test_sem_ic_2]************\n");
  int s;
  int i;
  for(i=0;;i++){
    if ((s=screate(i))==-1){
      break;
    }
  }
  ASSERT_DEBUG(TST_SEM, i==NB_SEM);
  s=sdelete(99);
  ASSERT_DEBUG(TST_SEM, s==0);
  s=sdelete(99);
  ASSERT_DEBUG(TST_SEM, s==-1);

  for(i=1;i<NB_SEM-1;i++){
    ASSERT_DEBUG(TST_SEM,((s=sdelete(i))==0));
  }
  return s;
}


// Test sdelete
int test_sem_d() {
  int s[5];
  PRINT_DEBUG(TST_SEM, 2, "*************[test_sem_d]*************\n");
  for(int i =0;i<5;i++){
    ASSERT_DEBUG(TST_SEM,((s[i]=screate(i))==(i+1)));
  }
  for(int i =0;i<5;i++){
    ASSERT_DEBUG(TST_SEM,((s[i]=sdelete(i))==0));
  }
  return 0;
}


// Test scount
int test_sem_c() {
  int s=screate(5);
  PRINT_DEBUG(TST_SEM, 2, "*************[test_sem_c]**************\n");
  ASSERT_DEBUG(TST_SEM,((scount(s))==5));
  sdelete(s);
  ASSERT_DEBUG(TST_SEM,((scount(s))==-1));
  return 0;
}


// Test sreset/count
int test_sem_wc() {
  int s=screate(5);
  PRINT_DEBUG(TST_SEM, 2, "*************[test_sem_sc]*************\n");
  ASSERT_DEBUG(TST_SEM,((scount(s))==5));
  sdelete(s);
  ASSERT_DEBUG(TST_SEM,((scount(s))==-1));
  s=screate(-1);
  ASSERT_DEBUG(TST_SEM,((scount(s))==-1));
  ASSERT_DEBUG(TST_SEM,((sdelete(s))==-1));
  s=screate(SHRT_MAX);
  ASSERT_DEBUG(TST_SEM,((scount(s))==SHRT_MAX));
  sreset(s, 5);
  ASSERT_DEBUG(TST_SEM,((scount(s))==5));
  ASSERT_DEBUG(TST_SEM,((sreset(s,0))==0));
  ASSERT_DEBUG(TST_SEM,((sreset(s,-1))==-1));
  ASSERT_DEBUG(TST_SEM,((sreset(45,-1))==-1));
  return 0;
}


/******************************************************
 * Tests avec variables partagees  (signaln, sdelete) *
 ******************************************************/
static int shared_var;

int test_sem_P_1() {
  PRINT_DEBUG(TST_SEM, 2, "***********[test_sem_P_1]************\n");
  shared_var=screate(0);
  int val_wait=wait(shared_var);
  int val_count=scount(shared_var);
  ASSERT_DEBUG(TST_SEM,val_count==-2);
  ASSERT_DEBUG(TST_SEM,val_wait==0);
  return 0;
}

int test_sem_P_2() {
  PRINT_DEBUG(TST_SEM, 2, "***********[test_sem_P_2]************\n");
  int val_count=scount(shared_var);
  int val_wait=wait(shared_var);
  ASSERT_DEBUG(TST_SEM,val_count==-1);
  ASSERT_DEBUG(TST_SEM,val_wait==0);
  return 0;
}

int test_sem_P_3() {
  PRINT_DEBUG(TST_SEM, 2, "***********[test_sem_P_3]************\n");
  int val_count=scount(shared_var);
  int val_wait=wait(shared_var);
  ASSERT_DEBUG(TST_SEM,val_count==-2);
  ASSERT_DEBUG(TST_SEM,val_wait==-3);
  return 0;
}


int test_sem_V_total() {
  PRINT_DEBUG(TST_SEM, 2, "*********[test_sem_V_total]**********\n");
  int val_count=scount(shared_var);
  ASSERT_DEBUG(TST_SEM,val_count==-3);
  signaln(shared_var, 2);
  val_count=scount(shared_var);
  ASSERT_DEBUG(TST_SEM,val_count==-1);
  sdelete(shared_var);
  return 0;
}




/****************************************************
 * Tests avec variables partagees  (signal, sreset) *
 ****************************************************/

int test_sem_P_2_1() {
  PRINT_DEBUG(TST_SEM, 2, "***********[test_sem_P_2_1]************\n");
  shared_var=screate(0);
  int val_count=scount(shared_var);
  int val_wait=wait(shared_var);
  PRINT_DEBUG(TST_SEM, 2, "[test_sem_P_2_1] se reveille\n");
  ASSERT_DEBUG(TST_SEM,val_wait==0);
  ASSERT_DEBUG(TST_SEM,val_count==0);
  return 0;
}

int test_sem_P_2_2() {
  PRINT_DEBUG(TST_SEM, 2, "***********[test_sem_P_2_2]************\n");
  int val_count=scount(shared_var);
  int val_wait=wait(shared_var);
  PRINT_DEBUG(TST_SEM, 2, "[test_sem_P_2_2] se reveille\n");
  ASSERT_DEBUG(TST_SEM,val_wait==0);
  ASSERT_DEBUG(TST_SEM,val_count==-1);
  return 0;
}

int test_sem_P_2_3() {
  PRINT_DEBUG(TST_SEM, 2, "***********[test_sem_P_2_3]************\n");
  int val_count=scount(shared_var);
  int val_wait=wait(shared_var);
  PRINT_DEBUG(TST_SEM, 2, "[test_sem_P_2_3] se reveille\n");
  ASSERT_DEBUG(TST_SEM,val_wait==0);
  ASSERT_DEBUG(TST_SEM,val_count==-2);
  return 0;
}

int test_sem_P_2_4() {
  PRINT_DEBUG(TST_SEM, 2, "***********[test_sem_P_2_4]************\n");
  int val_count=scount(shared_var);
  int val_wait=wait(shared_var);
  PRINT_DEBUG(TST_SEM, 2, "[test_sem_P_2_4] se reveille\n");
  ASSERT_DEBUG(TST_SEM,val_wait==-4);
  ASSERT_DEBUG(TST_SEM,val_count==-3);
  return 0;
}

int test_sem_V_1_3() {
  PRINT_DEBUG(TST_SEM, 2, "***********[test_sem_V_1_3]************\n");
  int val_count=scount(shared_var);
  ASSERT_DEBUG(TST_SEM,val_count==-4);
  for(int i =0;i<3;i++){
    signal(shared_var);
  }
  val_count=scount(shared_var);
  ASSERT_DEBUG(TST_SEM,val_count==-1);
  return 0;
}


int test_sem_V_4() {
  PRINT_DEBUG(TST_SEM, 2, "*************[test_sem_V_4]************\n");
  int val_count=scount(shared_var);
  ASSERT_DEBUG(TST_SEM,val_count==-1);
  sreset(shared_var,0);
  val_count=scount(shared_var);
  ASSERT_DEBUG(TST_SEM,val_count==0);
  return 0;
}



/**********************************************
 * Tests avec variables partagees  (try_wait) *
 **********************************************/


int test_sem_P_3_1() {
  PRINT_DEBUG(TST_SEM, 2, "***********[test_sem_P_3_1]************\n");
  shared_var=screate(0);
  int val_count=scount(shared_var);
  int val_wait=try_wait(shared_var);
  ASSERT_DEBUG(TST_SEM,val_wait==-3);
  ASSERT_DEBUG(TST_SEM,val_count==0);
  return 0;
}

int test_sem_P_3_2() {
  PRINT_DEBUG(TST_SEM, 2, "***********[test_sem_P_3_2]************\n");
  int val_count=scount(shared_var);
  int val_wait=try_wait(shared_var);
  ASSERT_DEBUG(TST_SEM,val_wait==-3);
  ASSERT_DEBUG(TST_SEM,val_count==0);
  return 0;
}

int test_sem_P_3_3() {
  PRINT_DEBUG(TST_SEM, 2, "***********[test_sem_P_3_3]************\n");
  int val_count=scount(shared_var);
  int val_wait=try_wait(shared_var);
  ASSERT_DEBUG(TST_SEM,val_wait==-3);
  ASSERT_DEBUG(TST_SEM,val_count==0);
  return 0;
}

int test_sem_V_3_1() {
  PRINT_DEBUG(TST_SEM, 2, "***********[test_sem_V_3_1]************\n");
  int val_count=scount(shared_var);
  ASSERT_DEBUG(TST_SEM,val_count==0);
  sreset(shared_var,0);
  val_count=scount(shared_var);
  ASSERT_DEBUG(TST_SEM,val_count==0);
  return 0;
}

/******************************************************************
 * Tests concurrence : Allocateur memoire + diner des philospohes *
 ******************************************************************/
#define NB_CASES 10
#define NB_THREADS 10

static int nb_resources = NB_CASES;
static int forks[NB_THREADS];
static int ord_fourch[NB_THREADS];
static int mutex;
static int shortage;

void allocate(int n){

  wait(mutex);

  // on teste le nombre de ressources disponibles
  while (n > nb_resources) {
    // si il n'y en pas assez : s'endormir
    PRINT_DEBUG(TST_SEM, 2, "thread %2d, se bloque\n",getpid());
    signal(mutex);
    wait(shortage);
    wait(mutex);
  }

  // si on arrive ici il y a assez de ressources : on les prend
  nb_resources -= n;
  PRINT_DEBUG(TST_SEM, 2, "thread %2d, alloue %2d\n",getpid(),n);
  signal(mutex);
}

void restore(int n){
  
  // on libere des ressources
  wait(mutex);
  nb_resources += n;
  PRINT_DEBUG(TST_SEM, 2, "thread %2d, libere %2d\n",getpid(),n);
  signal(mutex);

  // on signale que des ressources ont ete liberees
  sreset(shortage,1);
}









/*
 * Diner des philospohes avec un philosophe gaucher
 */
void take_fork(int i){
  if(i==NB_THREADS-1){
    PRINT_DEBUG(TST_SEM, 2, "Philosophe %d (gaucher) demande les fourchettes %d et %d\n",
	   getpid(),(i+1)%NB_THREADS,(i)%NB_THREADS);
    wait(forks[(i+1)%NB_THREADS]);
    wait(forks[(i)%NB_THREADS]);
  }
  else{  
    PRINT_DEBUG(TST_SEM, 2, "Philosophe %d (droitier) demande les fourchettes %d et %d\n",
	   getpid(),(i)%NB_THREADS,(i+1)%NB_THREADS);
    wait(forks[(i)%NB_THREADS]);
    wait(forks[(i+1)%NB_THREADS]);
  }
}

void put_fork(int i){
  if(i==NB_THREADS-1){
    PRINT_DEBUG(TST_SEM, 2, "Philosophe %d (gaucher) repose les fourchettes %d et %d\n",
	   getpid(),(i)%NB_THREADS,(i+1)%NB_THREADS);
    sreset(forks[(i+1)%NB_THREADS],1); 
    sreset(forks[(i)%NB_THREADS],1);
  }
  else{
    PRINT_DEBUG(TST_SEM, 2, "Philosophe %d (droitier) repose les fourchettes %d et %d\n",
	   getpid(),(i)%NB_THREADS,(i+1)%NB_THREADS);
    sreset(forks[(i)%NB_THREADS],1); 
    sreset(forks[(i+1)%NB_THREADS],1);
  }
}


int thread_phil(void * a){
  int v;
  for(int i=0;i<5;i++){
    v=my_rand();
    allocate(v);
    restore(v);
    sleep(rand_a_b(1, 5));
    take_fork(*((int *)a));
    put_fork(*((int *)a));
  }
  return 0;
}


int end_phil(){
  // Destruction des semaphores
  for(int i=0;i<NB_THREADS-1;i++){
    sdelete(forks[i]);
  }
  sdelete(shortage);
  sdelete(mutex);
  return 0;
}

int main_phil(){
  // Initialisation des variables
  
  for(int i=0;i<NB_THREADS;i++){
    ord_fourch[i]=i;
  }
  // Initialisation des semaphores
  shortage=screate(0);
  mutex=screate(1);
  for(int i=0;i<NB_THREADS-1;i++){
    forks[i]=screate(1);
  }

  for(int i=0;i<NB_THREADS-1;i++){
    start(&(thread_phil), 0x20000, 10, "thread_phil", &ord_fourch[i]);
  }
  start(&(end_phil), 0x1000, 1, "end_phil", 0);
  return 0;
}



/*
 * Lanceur de tests
 */
void test_sem(void) {
  printf("********** Tests semaphores **********\n\n");
  start(test_sem_cd, 0x20000, 30, "test_sem_cd", 0);
  start(test_sem_id, 0x20000, 29, "test_sem_id", 0);
  start(test_sem_ic_1, 0x20000, 28, "test_sem_ic_1", 0);
  start(test_sem_ic_2, 0x20000, 27, "test_sem_ic_2", 0);
  start(test_sem_d, 0x20000, 26, "test_sem_d", 0);
  start(test_sem_c, 0x20000, 25, "test_sem_c", 0);
  start(test_sem_wc, 0x20000, 24, "test_sem_wc", 0);
  start(test_sem_P_1, 0x20000, 23, "test_sem_P_1", 0);
  start(test_sem_P_2, 0x20000, 22, "test_sem_P_2", 0);
  start(test_sem_P_3, 0x20000, 21, "test_sem_P_3", 0);
  start(test_sem_V_total, 0x20000, 20, "test_sem_V_total", 0);
  start(test_sem_P_2_1, 0x20000, 19, "test_sem_P_2_1", 0);
  start(test_sem_P_2_2, 0x20000, 18, "test_sem_P_2_2", 0);
  start(test_sem_P_2_3, 0x20000, 17, "test_sem_P_2_3", 0);
  start(test_sem_P_2_4, 0x20000, 16, "test_sem_P_2_4", 0);
  start(test_sem_V_1_3, 0x20000, 15, "test_sem_V_1_3", 0);
  start(test_sem_V_4, 0x20000, 14, "test_sem_V_4", 0);
  start(test_sem_P_3_1, 0x20000, 13, "test_sem_P_3_1", 0);
  start(test_sem_P_3_2, 0x20000, 12, "test_sem_P_3_2", 0);
  start(test_sem_P_3_3, 0x20000, 11, "test_sem_P_3_3", 0);
  start(test_sem_V_3_1, 0x20000, 10, "test_sem_V_3_1", 0);
  start(main_phil, 0x20000, 9, "main_phil", 0);
}
