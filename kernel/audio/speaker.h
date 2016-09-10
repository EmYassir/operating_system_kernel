#ifndef __SPEAKER_H__
#define __SPEAKER_H__

void init_speaker(void);
void set_frequency(unsigned int freq);
void speaker_on(void);
void speaker_off(void);
void beep(unsigned int freq);

#endif /* __SPEAKER_H__ */
