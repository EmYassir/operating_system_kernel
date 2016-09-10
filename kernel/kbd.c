#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "private_kbd.h"
#include "kbd.h"
#include "console.h"
#include "process_handler.h"

/*******************************************************************************/
/*                              Variables globales                             */
/*******************************************************************************/
static int echo = 1;                      // Activation de l'echo sur la console
static int calling_pid = 0;               // Pid du processus appelant
static int reading = 0;                   // Indique si une lecture est en cours
static unsigned long nb_chars = 0;        // Nombre de caracteres dans le buffer


// Buffer
static frame buffer[BUFFER_SIZE]; 

// Table des deux buffers
static link queue_table[NB_QUEUES]={        // Files de caracteres :
  LIST_HEAD_INIT(queue_table[kbd_used]),    //  1) places occupees buffer clavier
  LIST_HEAD_INIT(queue_table[kbd_free]),    //  2) places libres buffer clavier
};

/*******************************************************************************/
/*                             Fonction externe utile                          */
/*******************************************************************************/

/* 
 * scancode
 */
extern void do_scancode(int scancode);

/*******************************************************************************/
/*                               Gestion echo ecran                            */
/*******************************************************************************/


/* 
 * Reglage echo sur la console
 */

void set_echo(int on){
  if(on){
    echo=1;
  }
  else{
    echo=0;
  }
}


/*******************************************************************************/
/*                                Gestion buffers                              */
/*******************************************************************************/
/* 
 * Initialisation buffers
 */

void init_buffer(void){
  nb_chars=0;
  for (int i=0;i<BUFFER_SIZE;i++){
    buffer[i].id = i;
    buffer[i].val = '\0';
    buffer[i].arrival = 1;
    buffer[i].chaining.next = 0;
    buffer[i].chaining.prev = 0;
    queue_add(&buffer[i],
	      &queue_table[kbd_free], 
	      frame, 
	      chaining, 
	      id);
  }
} 

/* 
 * Remise a zero du buffer
 */
void update_buffer(void){
  nb_chars=0;
  while(!queue_empty(&queue_table[kbd_used])){
    remove_buff();
  }
} 

/*
 * Buffer vide
 */
int buff_empty(void){
  return (queue_empty(&queue_table[kbd_used]));
}

/*
 * Supression du buffer
 */
char remove_buff(void) {    
  frame * cur=queue_out(&queue_table[kbd_used], frame, chaining);
  if(cur!=0){
    queue_add(cur,&queue_table[kbd_free],frame,chaining,id);
    nb_chars--;
    return cur->val;
  }
  else{
    return '\0';
  }
}


/*
 * Acquisition id libre
 */
static int obtain_id(void) {
  if (queue_empty(&queue_table[kbd_free])){
    return -1;
  }
  return (queue_out_bot(&queue_table[kbd_free], frame, chaining))->id;
} 

/*******************************************************************************/
/*                                Gestion lecture                              */
/*******************************************************************************/

/* 
 * Demarrage lecture
 */
void enable_reading(int pid){
  calling_pid = pid; 
  reading = 1;
}



/* 
 * Recopie a partir du tampon
 */
unsigned long copy_from_buffer(char * str,unsigned long length){
  char c;
  unsigned long length_read = 0;
  while(1){
    if (length_read==length){
      str[length_read]='\0';
      return length_read;      
    }
    if(buff_empty()){
      str[length_read]='\0';
      return length_read;
    }
    c=remove_buff();
    if(c=='\0'){
      str[length_read]='\0';
      return length_read;
    }
    else if((c>=0)&&(c<=126)){
      str[length_read]=c;
      length_read++;
    }
  }
  return length_read;
}


/*******************************************************************************/
/*                                Gestion clavier                              */
/*******************************************************************************/

/*
 * Gestion des leds du clavier
 */
void kbd_leds(unsigned char leds)
{
  outb(0xED, 0x60);
  outb(leds, 0x60);
}

/*
 * Gestion du clavier
 */
void keyboard_data(char *str)
{
  // Affichage a l'ecran  
  if(echo==1){
    if (str[0]!=13){
      if((0<=str[0])&&(str[0]<32)){
	handle_char('^');
	handle_char(str[0]+64);
      }
      else{
	handle_char(str[0]);
      }
    }
  }

  // Demande de lecture
  if(reading){
    
    // Gestion des touches entrees
    if(str[0]==13){ 
   
      /******************** Insertion buffer et fin lecture ********************/
      int id = obtain_id();
      if(id>=0){
	nb_chars++;
	buffer[id].val='\0';  
	queue_add(&buffer[id],
		  &queue_table[kbd_used], 
		  frame, 
		  chaining, 
		  arrival);
      }

      int pid = calling_pid;
      reading = 0;
      calling_pid=0;
      signal_end_io(pid);
      /*************************************************************************/ 
    }    
    // Supression du buffer
    else if (str[0]==127){

      /************************** Suppression du buffer ************************/            
      frame * cur=queue_out_bot(&queue_table[kbd_used], frame, chaining);
      if(cur!=0){
	queue_add(cur,&queue_table[kbd_free],frame,chaining,id);
	nb_chars--;

      }
      /*************************************************************************/ 
    }
    else{
      /************************* Insertion dans le buffer **********************/ 
      int id = obtain_id();
      if(id>=0){
	nb_chars++;
	buffer[id].val=str[0];  
	queue_add(&buffer[id],
		  &queue_table[kbd_used], 
		  frame, 
		  chaining, 
		  arrival);
      }
      /*************************************************************************/     
    }
  }
}
