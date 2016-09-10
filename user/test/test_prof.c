/*
 * test_prof.c
 *
 * Copyright (C) 2012 by group psys-7
 *
 * Wrapper pour le pilote de test fourni par les professeurs.
 */

#include "test_all.h"
#include "test.h"
#include "syslib.h"
#include "cmd_lib.h"

extern int test_proc(void *);

void sys_info(void)
{
	// liste des processus
	shell_ps();

	// liste des sémaphores
	shell_sinfo();
}

void test_prof(void)
{
	start(&test_proc, 0x1000, 128, "test_proc", 0);
}
