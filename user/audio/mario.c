#define fa3	349
#define la3	440
#define do4	523
#define si3b	466
#define si3	494
#define fa4	698
#define re4	587
#define mi4	659
#define do4d	554
#define mi3	330
#define sol3	392

#define nop	0

#include "mario.h"

unsigned int mario_notes[81] = {
	la3, la3, nop, la3, nop, fa3, la3, nop, do4, nop, nop, nop, nop, nop, nop, nop,
	do4, nop, nop, la3, nop, nop, fa3, nop, nop, nop, si3b, nop, do4, nop, si3, si3b, nop, nop,
	do4, nop, nop, la3, nop, nop, fa3, nop, nop, nop, si3b, nop, do4, nop, si3, si3b, nop, nop,
	fa4, nop, nop, do4, nop, nop, la3, nop, re4, nop, mi4, re4, do4d, nop,
	do4, la3, do4, re4, nop, nop, si3b, do4, nop, la3, nop, fa3, sol3, mi3, nop};

track mario = { &(mario_notes[0]), 81};
