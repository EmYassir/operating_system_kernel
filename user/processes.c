#include "processes.h"
#include "syslib.h"
#include "utils.h"
#include "stdio.h"
#include "cmd_lib.h"
#include "audio_format.h"
#include "mario.h"

int banner()
{
	clear();
	printf("\n");
	printf("        _                                                  _    ___    ____  \n");
	printf("       / \\     _ __     ___    _ __    _   _   _ __ ___   ( )  / _ \\  / ___| \n");
	printf("      / _ \\   | '_ \\   / _ \\  | '_ \\  | | | | | '_ ` _ \\  |/  | | | | \\___ \\ \n");
	printf("     /  _  \\  | | | | | (_) | | | | | | |_| | | | | | | |     | |_| |  ___) |\n");
	printf("    /_/   \\_\\ |_| |_|  \\___/  |_| |_|  \\__, | |_| |_| |_|      \\___/  |____/ \n");
	printf("                                       |___/                                 \n");
	printf("\n");
	printf("    Projet de specialite - Projet Systeme\n");
	printf("    Ensimag 2012\n");
	printf("                                                               Raphael Bleuse\n");
	printf("                                                            Yassir El Mesbahi\n");
	printf("                                                               Nicolas Perrot\n");
	return 0;
}

int audio()
{
	play_music(&mario);
	return 0;
}

extern int test_proc(void *);

int tests()
{
	return start(&test_proc, 0x1000, 128, "test_proc", 0);
}
