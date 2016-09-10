#ifndef __INTERRUPTION_HANDLERS_H__
#define __INTERRUPTION_HANDLERS_H__

// Traitant de l'interruption horloge, interruption 32
extern void it32_clock_handler(void);

// Traitant de l'interruption des appels aux fonctions système, interruption 49
extern void it49_syslib_handler(void);

// Traitant des exceptions liees a des instructions interdites, interruption 13
extern void it13_exception_handler(void);

// Traitant des interruptions clavier, interruption 33
extern void it33_kbd_handler(void);

// Traitant de l'interruption RTC, interruption 40
extern void it40_rtc_handler(void);

#endif /* __INTERRUPTION_HANDLERS_H__ */
