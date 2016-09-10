/*
 * test.h
 *
 * Copyright (C) 2012 by Raphaël Bleuse
 *
 * Base de tests pour le noyau.
 */

#ifndef __KERNEL_TEST_H__
#define __KERNEL_TEST_H__

/******************************************************************************/
/* Définition des tests à effectuer                                           */
/*                                                                            */
/* Définir pour chaque test le niveau de détail :                             */
/* - 0 le test n'est pas effectué.                                            */
/* - >0 le test est effectué avec le niveau de détail spécifié.               */
/******************************************************************************/

/* Test des profs : commenter pour ne pas les faire */
#define TST_PROF

#define TST_START	0
#define TST_EXIT	0
#define TST_KILL	0
#define TST_WAIT_CLOCK	0
#define TST_PRIO	0
#define TST_FILIATION	0
#define TST_SEM		0
#define TST_CONS_READ	0
#define TST_MEMORY	0

/*----------------------------------------------------------------------------*/

/* Fonction de test */

void test_all(void);

/*----------------------------------------------------------------------------*/

/******************************************************************************/
/* Macro d'impression des tests                                               */
/*                                                                            */
/* test :  nom du test concerné par la trace.                                 */
/* level : niveau de détail de la trace.                                      */
/* args :  arguments du printf à effectuer pour la trace.                     */
/******************************************************************************/
#define PRINT_DEBUG(test,level,args...) do {\
	if (test >= level) {\
		printf(args);\
	}\
} while(0)

/******************************************************************************/
/* Macro d'assertion.                                                         */
/*                                                                            */
/* test : nom du test concerné par la trace.                                  */
/* args : arguments du assert.                                                */
/******************************************************************************/
#define ASSERT_DEBUG(test,args...) do {\
	if (test > 0) {\
		assert(args);\
	}\
} while (0)

#endif
