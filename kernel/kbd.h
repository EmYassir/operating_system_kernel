/*
 * kbd.h
 *
 * Copyright (C) 2005 by Simon Nieuviarts
 *
 * Keyboard scancode handling.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __KBD_H__
#define __KBD_H__


/*******************************************************************************/
/*                               Gestion echo ecran                            */
/*******************************************************************************/

/* 
 * Reglage echo sur la console
 */
void set_echo(int on);

/*******************************************************************************/
/*                                Gestion buffers                              */
/*******************************************************************************/

/* 
 * Initialisation buffers
 */
void init_buffer(void);

/* 
 * Remise a zero du buffer
 */
void update_buffer(void);
 
/*
 * Buffer vide
 */
int buff_empty(void);

/*
 * Supression du buffer
 */
char remove_buff(void);



#define PRINT_ERROR_OVERFLOW() do {\
    printf("KEYBOARD BUFFER OVERFLOW : tape <Enter> to escape\n");\
} while(0)

/*******************************************************************************/
/*                                Gestion lecture                              */
/*******************************************************************************/

/* 
 * Indicateur de lecture
 */
void enable_reading(int pid);


/* 
 * Recopie a partir du tampon
 */
unsigned long copy_from_buffer(char * str,unsigned long length);


/*******************************************************************************/
/*                                Gestion clavier                              */
/*******************************************************************************/


/*
 * Gestion des leds du clavier
 */
void kbd_leds(unsigned char leds);

/*
 * Gestion du clavier
 */
void keyboard_data(char *str);

#endif
