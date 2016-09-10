/*
 * test_all.c
 *
 * Copyright (C) 2012 by group psys-7
 *
 * Base de tests pour le noyau.
 */

#include "test.h"
#include "test_all.h"

void test_all(void)
{

#ifdef TST_PROF
	test_prof();
#else /* TST_PROF */

#if TST_START
	test_start();
#endif /* TST_START */

#if TST_EXIT
	test_exit();
#endif /* TST_EXIT */

#if TST_KILL
	test_kill();
#endif /* TST_KILL */

#if TST_WAIT_CLOCK
	test_wait_clock();
#endif /* TST_WAIT_CLOCK */

#if TST_PRIO
	test_prio();
#endif /* TST_PRIO */

#if TST_FILIATION
	test_filiation();
#endif /* TST_FILIATION */

#if TST_SEM
	test_sem();
#endif /* TST_SEM */

#if TST_CONS_READ
	test_cons_read();
#endif /* TST_SEM */

#endif /* TST_PROF */
}
