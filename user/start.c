#include "processes.h"
#include "syslib.h"
#include "shell.h"
#include "test.h"

/* The user code entry point */
int user_start(void * args) {
	start(audio, 0x1000, 255, "audio", 0);
	start(banner, 0x1000, 255, "banner", 0);
	test_all();
	//start(&(start_shell), 0x1000, 5, "start_shell", 0);
	return 0;
}
