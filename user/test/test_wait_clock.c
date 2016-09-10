/*
 * test_wait_clock.c
 *
 * Copyright (C) 2012 by group psys-7
 *
 * Base de tests pour wait_clock & sleep.
 */

#include "test_all.h"
#include "test.h"
#include "syslib.h"
#include "stdio.h"

int star_periode_1(void *arg) {
	int i;
	for (i=0; i < 40; i++) {
		PRINT_DEBUG(TST_WAIT_CLOCK, 2, "*");
		wait_clock(10);
	}
	return 0;
}

int point_periode_2(void *arg) {
	int i;
	for (i=0; i < 20; i++) {
		PRINT_DEBUG(TST_WAIT_CLOCK, 2, ".");
		wait_clock(20);
	}
	return 0;
}

int dollar_periode_4(void *arg) {
	int i;
	for (i=0; i < 10; i++) {
		PRINT_DEBUG(TST_WAIT_CLOCK, 2, "$");
		wait_clock(40);
	}
	return 0;
}

void test_wait_clock(void) {
	start(&star_periode_1, 0x20000, 1, "disp *", 0);
	start(&point_periode_2, 0x20000, 1, "disp .", 0);
	start(&dollar_periode_4, 0x20000, 1, "disp $", 0);
}
