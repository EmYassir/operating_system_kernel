/*
 * test_filiation.c
 *
 * Copyright (C) 2012 by group psys-7
 *
 * Base de tests pour la filiation.
 */

#include "test_all.h"
#include "test.h"
#include "syslib.h"
#include "stdio.h"

int son(void *arg) {
	return getpid();
}

int unique_father(void *arg) {
	int retval_son;
	int pid_son;

	PRINT_DEBUG(TST_FILIATION, 2, "[unique_father] creating a son\n");
	pid_son = start(&son, 0x1000, 1, "unique son", 0);
	PRINT_DEBUG(TST_FILIATION, 2, "[unique_father] son has pid %i, waiting for him\n", pid_son);
	waitpid(-1, &retval_son);
	
	PRINT_DEBUG(TST_FILIATION, 2, "[unique_father] son with pid %i tells me he has pid %i\n", pid_son, retval_son);
	ASSERT_DEBUG(TST_FILIATION, retval_son == pid_son);

	return 0;
}

int father_of_many(void *arg) {

	int ret;
	int waited_son;
	int son_pid;

	for (int i = 0; i < 5; i++) {
		PRINT_DEBUG(TST_FILIATION, 2, "[father_of_many] trying to start %i son\n", i);
		son_pid = start(&son, 0x1000, 1, "son_", 0);
		if (son_pid != -1) {
			PRINT_DEBUG(TST_FILIATION, 2,"[father_of_many] son %i creation succeeded : pid %i\n", i, son_pid);
		} else {
			PRINT_DEBUG(TST_FILIATION, 2,"[father_of_many] son %i creation failed\n", i);
		}
	}

	PRINT_DEBUG(TST_FILIATION, 2, "[father_of_many] waiting son with pid %i\n", son_pid);
	waited_son = waitpid(son_pid, &ret);
	PRINT_DEBUG(TST_FILIATION, 2, "[father_of_many] son with pid %i died\n", son_pid);
	ASSERT_DEBUG(TST_FILIATION, ret == waited_son);
	return 0;
}

void test_filiation(void) {
	start(&unique_father, 0x1000, 1, "unique_father", 0);
	start(&father_of_many, 0x1000, 1, "father_of_many", 0);
}
