#include "audio.h"
#include "speaker.h"
#include "interruption.h"
#include "port.h"
#include "cpu.h"

/* Variables globales */
static track *cur_track;	/* morceau en cours de lecture */
static unsigned long cur_note;	/* note du morceau en cours */

/*
 * Programmation du RTC.
 */
void init_rtc(void) {
	/*
	 * Programmation de l'IRQ8.
	 *
	 * - NMI masqué par bit[7] à 0 pendant les écritures sur CMOS_ADDRESS
	 * - les interruptions périodiques sont activés par le bit 6 du registre
	 *   B.
	 * - la frequence est mise à 8kHz. Paramétré par les bits[3-0] du
	 *   registre A.
	 *
	 * Plus de détail: http://wiki.osdev.org/RTC#Turning_on_IRQ_8
	 *                 http://www.bioscentral.com/misc/cmosmap.htm
	 */

	char tmp;

	// interruptions périodiques
	outb(0x0B, CMOS_ADDRESS);
	tmp = inb(CMOS_DATA);
	outb(0x0B, CMOS_ADDRESS);
	outb(tmp | 0x40, CMOS_DATA);

	// fréquence de 8kHz
	outb(0x0A, CMOS_ADDRESS);
	tmp = inb(CMOS_DATA);
	outb(0x0A, CMOS_ADDRESS);
	outb((tmp & 0xF0) | 0xD, CMOS_DATA);
}

/*
 * Initialisation de la couche audio.
 * Initialise le haut parleur & le RTC.
 */
void init_audio(void) {
	init_speaker();
	init_rtc();
}

/*
 * Démarre la lecture d'un morceau de musique.
 */
void play_music(track *music)
{
	// on recupere le morceau à jouer
	cur_track = music;
	cur_note = 1;

	set_frequency(cur_track->notes[0]);
	speaker_on();

	// on change de note sur chaque IRQ8 (IRQ2 pour chaînage)
	mask_IRQ(2, 0);
	mask_IRQ(8, 0);
}

/*
 * Arrête de jouer la piste.
 */
void stop_music(void)
{
	speaker_off();
	mask_IRQ(2,1);
	mask_IRQ(8, 1);
}

/*
 * Change la note jouée par le haut parleur.
 * Appelé par le traitant de l'interruption RTC (#40).
 */
void next_note(void)
{
	if (cur_note < cur_track->length) {
		if (cur_track->notes[cur_note] != 0) {
			set_frequency(cur_track->notes[cur_note++]);
			speaker_on();
		} else {
			cur_note++;
			speaker_off();
		}
	} else {
		stop_music();	
	}
}
