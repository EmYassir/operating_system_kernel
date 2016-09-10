#ifndef __TIMER_H__
#define __TIMER_H__

#define CLOCK_CT 7
#define CLOCK_CF 0

/*
 * Primitives
 */

void clock_settings(unsigned long *quartz, unsigned long *ticks);
unsigned long current_clock();

void init_clock(void);
void increment_clock(void);
void refresh_clock(void);

#endif
