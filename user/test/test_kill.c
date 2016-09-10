/*
 * test_kill.c
 *
 * Copyright (C) 2012 by group psys-7
 *
 * Base de tests pour kill.
 */

#include "test_all.h"
#include "test.h"
#include "syslib.h"
#include "stdio.h"

int innocent_process(void *arg) {
	for(;;) {
	}
	return 1;
}

int serial_killer(void *arg) {
	int kill_ret_code;
	if ((int)arg == -1) {
		PRINT_DEBUG(TST_KILL, 2, "[serial_killer] suicide\n");
		kill(getpid());
	} else {
		PRINT_DEBUG(TST_KILL, 2, "[serial_killer] murder attempt on %i\n", (int)arg);
		kill_ret_code = kill((int)arg);
		ASSERT_DEBUG(TST_KILL, kill_ret_code <= 0);
		if (kill_ret_code == 0) {
			PRINT_DEBUG(TST_KILL, 2, "[serial_killer] %i died\n", (int)arg);
		} else {
			PRINT_DEBUG(TST_KILL, 2, "[serial_killer] %i survived\n", (int)arg);
		}
	}
	return 2;
}

int multi_frag(void *arg) {
	int pid1, pid2, pid3;

	pid1 = start(&innocent_process, 0x20000, 1, "innocent pid1", 0);
	pid2 = start(&innocent_process, 0x20000, 1, "innocent pid2", 0);
	pid3 = start(&innocent_process, 0x20000, 1, "innocent pid3", 0);

	// TODO -- ajouter traces

	kill(pid1);
	kill(pid2);
	kill(pid3);

	return 0;

}

void test_kill(void) {

	// some innocents to be killed
	int first_innocent = start(&innocent_process, 0x20000, 1, "first_innocent", 0);
	int second_innocent = start(&innocent_process, 0x20000, 1, "second_innocent", 0);
	int third_innocent = start(&innocent_process, 0x20000, 1, "third_innocent", 0);

	// several killers == one serial killer
	start(&serial_killer, 0x20000, 5, "serial_killer", (void *)first_innocent);
	start(&serial_killer, 0x20000, 5, "serial_killer", (void *)second_innocent);
	start(&serial_killer, 0x20000, 5, "serial_killer", (void *)third_innocent);
	start(&serial_killer, 0x20000, 5, "serial_killer", 0);
	
	// trying to kill a bad pid (inexistant one)
	start(&serial_killer, 0x20000, 5, "serial_killer", (void *)(-10));

	// one suicide
	start(&serial_killer, 0x20000, 5, "serial_killer", (void *)(-1));

	// one multi_frag
	start(&multi_frag, 0x20000, 1, "multi_frag", 0);
}
