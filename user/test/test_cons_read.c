/*
 * test_start.c
 *
 * Copyright (C) 2012 by group psys-7
 *
 * Base de tests pour start.
 */

#include "test_all.h"
#include "test.h"
#include "syslib.h"
#include "cmd_lib.h"
#include "stdio.h"
#include "string.h"

// Tableau d'ecriture
static char nom[30];

//Resultat
static unsigned long res;

int test_null(void *arg) {
  res = cons_read(nom,0);
  ASSERT_DEBUG(TST_CONS_READ, res==0);
  return 0;
}

int test_smaller_1(void *arg) {
  update_buffer();
  printf("Tapez \"\"\n");
  cons_read(nom,29);
  printf("vous avez tape : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom,"")==0);
  return 0;
}


int test_smaller_2(void *arg) {
  update_buffer();
  printf("Tapez \"toto\"\n");
  cons_read(nom,30);
  printf("vous avez tape : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom,"toto")==0);
  return 0;
}

int test_smaller_3(void *arg) {
  update_buffer();
  printf("Tapez \"toto\"\n");
  cons_read(nom,5);
  printf("vous avez tape : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom,"toto")==0);
  return 0;
}

int test_equal_1(void *arg) {
  update_buffer();
  printf("Tapez \"\"\n");
  cons_read(nom,1); 
  printf("vous avez tape : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom,"")==0);
  return 0;
}

int test_equal_2_3(void *arg) {
  update_buffer();
  printf("Tapez \"A\"\n");
  cons_read(nom,1); 
  printf("vous avez tape : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom,"A")==0);

  // Test prochain appel a "cons_read"
  cons_read(nom,1);
  printf("vous avez recupere : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom,"")==0);
  return 0;
}


int test_equal_4(void *arg) {
  update_buffer();
  // Reponse : "anticonstituellement"
  printf("Tapez \"anticonstituellement\"\n");
  cons_read(nom,20); 
  printf("vous avez tape : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom,"anticonstituellement")==0);
  cons_read(nom,1);
  printf("vous avez recupere : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom,"")==0);
  return 0;
}

int test_equal_5(void *arg) {
  update_buffer();
  printf("Tapez \"toto\"\n");
  cons_read(nom,4);
  printf("vous avez tape : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom,"toto")==0);
  cons_read(nom,1);
  printf("vous avez recupere : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom,"")==0);
  return 0;
}

int test_bigger_1(void *arg) {
  // Reponse : "anticonstituellement"
  update_buffer();
  printf("Tapez \"Tres grand\"\n");
  res = cons_read(nom,5); 
  printf("vous avez tape : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom,"Tres ")==0);
  ASSERT_DEBUG(TST_CONS_READ, res==5);
  // Test prochain appel a "cons_read"
  res=cons_read(nom,6);
  printf("vous avez recupere : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom,"grand")==0);
  ASSERT_DEBUG(TST_CONS_READ, res==5);
  return res;
}

int test_bigger_2(void *arg) {
  // Reponse : "anticonstituellement"
  update_buffer();
  printf("Tapez \"Petit Lion deviendra grand\"\n");
  res = cons_read(nom,5);
  printf("vous avez tape : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom,"Petit")==0);
  ASSERT_DEBUG(TST_CONS_READ, res==5);

  // Test prochains appels a "cons_read"
  res=cons_read(nom,6);
  printf("vous avez recupere : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom," Lion ")==0);
  ASSERT_DEBUG(TST_CONS_READ, res==6);

  // Test prochains appels a "cons_read"
  res=cons_read(nom,9);
  printf("vous avez recupere : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom,"deviendra")==0);
  ASSERT_DEBUG(TST_CONS_READ, res==9);

  // Test prochains appels a "cons_read"
  res=cons_read(nom,6);
  printf("vous recupere : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom," grand")==0);
  ASSERT_DEBUG(TST_CONS_READ, res==6);

  // Test prochains appels a "cons_read"
  res=cons_read(nom,6);
  printf("vous avez recupere : %s\n",nom);
  ASSERT_DEBUG(TST_CONS_READ, strcmp (nom,"")==0);
  ASSERT_DEBUG(TST_CONS_READ, res==0);
  return res;
}

void test_cons_read(void) {
  printf("**** Tests cons_read ****\n");
  printf(" Consigne : Tapez les nom entre \"\" demandes par le programme,\n"); 
  printf("suivis de la touche <ENTREE>\n");
  //start(test_null, 0x1000, 40, "test_null", 0);
  //start(test_smaller_1, 0x20000, 39, "test_smaller_1", 0);
  //start(test_smaller_2, 0x20000, 38, "test_smaller_2", 0);
  //start(test_smaller_3, 0x20000, 37, "test_smaller_3", 0);
  //start(test_equal_1, 0x20000, 36, "test_equal_1", 0);
  //start(test_equal_2_3, 0x20000, 35, "test_equal_2_3", 0);
  //start(test_equal_4, 0x20000, 33, "test_equal_4", 0);
  //start(test_equal_5, 0x20000, 32, "test_equal_5", 0);
  //start(test_bigger_1, 0x20000, 29, "test_bigger_1", 0);
  start(test_bigger_2, 0x20000, 28, "test_bigger_2", 0);
}
