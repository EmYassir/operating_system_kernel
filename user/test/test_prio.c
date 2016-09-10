/*
 * test_prio.c
 *
 * Copyright (C) 2012 by group psys-7
 *
 * Base de tests pour la gestion des priorités.
 */

#include "test_all.h"
#include "test.h"
#include "syslib.h"
#include "stdio.h"

int look_my_prio(void *arg) {
	PRINT_DEBUG(TST_PRIO, 3, "[look_my_prio] arg = %i\n", (int)arg);
	PRINT_DEBUG(TST_PRIO, 3, "[look_my_prio] prio = %i\n", getprio(getpid()));
	ASSERT_DEBUG(TST_PRIO, (int)arg == getprio(getpid()));
	return 0;
}

int look_inexistant_prio(void *arg) {
	int bad_prio = getprio(-1);
	PRINT_DEBUG(TST_PRIO, 2, "[look_inexistant_prio] getprio(-1) return %i\n", bad_prio);
	ASSERT_DEBUG(TST_PRIO, bad_prio < 0);
	return 0;
}

int nested_prio(void *arg) {
	if (start(&nested_prio, 0x1000, getprio(getpid()) + 10, "nested_prio", 0) > 0) {
		PRINT_DEBUG(TST_PRIO, 3, "%i > ", getprio(getpid()));
	}
	return 0;
}

void test_prio(void) {
	start(&look_my_prio, 0x20000, 42, "look_my_prio_42", (void *)42);
	start(&look_my_prio, 0x20000, 100, "look_my_prio_100", (void *)100);
	
	start(&look_inexistant_prio, 0x20000, 100, "look_inexistant_prio", 0);

	start(&nested_prio, 0x1000, 150, "nested_prio", 0);
}
