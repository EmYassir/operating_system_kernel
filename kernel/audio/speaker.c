#include "speaker.h"
#include "cpu.h"
#include "common.h"
#include "port.h"

void init_speaker(void)
{
	/*
	 * 0xB6 :
	 * -------------------------------------------------------------
	 * channel :		2
	 * access mode :	lobyte / hibyte
	 * op. mode :		square wave generator	
	 * mode :		16-bit binary
	 *
	 * plus de detail :
	 * http://wiki.osdev.org/Programmable_Interval_Timer#I.2FO_Ports
	 */
	outb(0xB6, PIT_CMD);
}

void set_frequency(unsigned int freq)
{
	unsigned int div = QUARTZ / freq;

	outb(div & 0xFF, PIT2_DATA);		// lobyte
	outb((div >> 8) & 0xFF, PIT2_DATA);	// hibyte
}

/* 
 * Le haut parleur est piloté par modulation de largeur d'impulsion (PWM).
 *
 * Les 2 bits de poids faible sont utilisés pour piloter le haut parleur :
 * - le bit 0 autorise le PIT à piloter le haut parleur.
 * - le bit 1 active / désactive le haut parleur.
 * Les bits sont actifs à l'état haut.
 */
void speaker_on(void)
{
	outb(inb(SPEAKER) | 0x03, SPEAKER);
}

void speaker_off(void)
{
	outb(inb(SPEAKER) & 0xFC, SPEAKER);
}

/*
 * Emettre un court beep à la fréquence freq.
 */
void beep(unsigned int freq)
{
	set_frequency(freq);
	speaker_on();
	for (int i = 0; i < 0x8000000; ++i); // TODO - remplacer par un sleep ?
	speaker_off();
}
