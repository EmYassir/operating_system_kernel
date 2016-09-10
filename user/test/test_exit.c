/*
 * test_exit.c
 *
 * Copyright (C) 2012 by group psys-7
 *
 * Base de tests pour exit.
 */

#include "test_all.h"
#include "test.h"
#include "syslib.h"
#include "stdio.h"

int explicit_exit_call(void *arg) {
	PRINT_DEBUG(TST_EXIT, 3, "%i should exit with 42\n", getpid());
	exit(42);
	return 2;
}

int implicit_exit_call(void *arg) {
	PRINT_DEBUG(TST_EXIT, 3, "%i should exit with 42\n", getpid());
	return 42;
	exit(2);
}

void test_exit(void) {
	start(explicit_exit_call, 0x20000, 1, "explicit_exit_call", 0);
	start(implicit_exit_call, 0x20000, 1, "implicit_exit_call", 0);
}
