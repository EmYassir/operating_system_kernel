#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

void init_handler_IT(int num_IT, void (*handler)(void),int user);
void mask_IRQ(int num_IRQ, int mask);

#endif
