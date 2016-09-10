#include "../shared/debug.h"
#include "common.h"
#include "cpu.h"
#include "console.h"
#include "timer.h"
#include "port.h"

static unsigned long clock_ticks;	// nb. interruptions horloge init_clock
static unsigned char second;
static unsigned char minute;
static unsigned int hour;

void init_clock() {
	// initialisation des variables
	clock_ticks = 0;
	second = 0;
	minute = 0;
	hour = 0;

	refresh_clock();

	// configuration du timer
	outb(0x34, PIT_CMD);				// change freq.
	outb((QUARTZ / CLOCKFREQ) & 0xFF, PIT0_DATA);	// poids faible
	outb((QUARTZ / CLOCKFREQ) >> 8, PIT0_DATA);	// poids fort
}

/*
 * current_clock : Heure actuelle
 *
 * Renvoie le nombre d'interruptions du timer depuis init_clock.
 */
unsigned long current_clock(void) {
	return clock_ticks;
}

/* Retourne dans *quartz la fréquence du quartz du système et dans *ticks le nombre d'oscillations du quartz entre chaque interruption */
void clock_settings(unsigned long *quartz, unsigned long *ticks){
	*quartz = QUARTZ;
	*ticks = QUARTZ/CLOCKFREQ;
}

void increment_clock(void) {
	clock_ticks++;
	if (clock_ticks % CLOCKFREQ == 0) { // 1 sec. de plus
		if ((second = (second + 1) % 60) == 0) {
			if ((minute = (minute + 1) % 60) == 0) {
				hour++;
			}
		}
		refresh_clock();
	}
}

/* 
 * refresh_clock : Rafraichissement de l'affichage de l'horloge
 *
 * on affiche l'horloge en haut à droite d'une ligne vide
 */
void refresh_clock(void) {
	// sauvegarde des paramètres de la console	
	int old_line = get_line_cur();
	int old_column = get_col_cur();
	int old_ct = get_ct_cur();
	int old_cf = get_cf_cur();

	// affichage
	set_ct_cur(CLOCK_CT);
	set_cf_cur(CLOCK_CF);
	move_cursor(0, NB_COLUMNS - 8);
	printf("%.2u:%.2u:%.2u", hour, minute, second);

	// restauration des paramètres de la console
	move_cursor(old_line, old_column);
	set_ct_cur(old_ct);
	set_cf_cur(old_cf);
}
