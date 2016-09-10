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
#include "stdio.h"
#include "../../kernel/common.h"

int should_be_42(void *arg) {
	ASSERT_DEBUG(TST_START, (int)arg == 42);
	return 0;
}

int use_8_stack(void *arg) {
	int local1 = 13;
	int local2 = 29;
	return local1 + local2;
}

void test_start(void) {
	int ret_code;

	// test passage d'argument
	ret_code = start(should_be_42, 0x20000, 1, "test_start", (void *)42);
	PRINT_DEBUG(TST_START, 2, "[test_start] start returned %i\n", ret_code);
	ASSERT_DEBUG(TST_START, ret_code > 0);

	// test utilisation pile
	ret_code = start(use_8_stack, 0x20000, 1, "use_8_stack_OK", 0);
	PRINT_DEBUG(TST_START, 2, "[test_start] start returned %i\n", ret_code);
	ASSERT_DEBUG(TST_START, ret_code > 0);

	// test start ave une trop grande priorité
	ret_code = start(should_be_42, 0x20000, MAXPRIO + 1, "priority too high", 0);
	PRINT_DEBUG(TST_START, 2, "[test_start] start returned %i\n", ret_code);
	ASSERT_DEBUG(TST_START, ret_code < 0);

	// test start ave une trop petite priorité
	ret_code = start(should_be_42, 0x20000, 0, "priority too low", 0);
	PRINT_DEBUG(TST_START, 2, "[test_start] start returned %i\n", ret_code);
	ASSERT_DEBUG(TST_START, ret_code < 0);
}
