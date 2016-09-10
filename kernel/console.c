#include <string.h>
#include "cpu.h"
#include "console.h"
/*
 * Variables globales
 */
static const int first_line = FIRST_LINE; //l'horloge occupe la premiere ligne, on commence alors l'affichage à partir de la deuxieme
static int line_cur = 0;                  // Ligne courante
static int col_cur = 0;                   // Colonne courante
static char ct_cur = 15;                  // Couleur de texte courante
static char cf_cur = 0;                   // Couleur de fond courante
static int  col_min = 0;                  // Colonne minimale
//static unsigned int tab_lines[NB_LINES];  // Tableau de colones maximales atteintes
/*
 * Accesseurs
 */

/* Ligne courante */
int get_line_cur(){
	return line_cur;
}

/* Colonne courante */
int get_col_cur(){
	return col_cur;
}

/* Colonne minimale */
int get_col_min(){
	return col_min;
}
/* Couleur de texte courante */
char get_ct_cur(){
	return ct_cur;
}

/* Couleur de fond courante */
char get_cf_cur(){
	return cf_cur;
}


/*
 * Mutateurs
 */

/* Ligne courante */
void set_line_cur(int line){
	line_cur=line;
}

/* Colonne courante */
void set_col_cur(int col){
	col_cur=col;
}

/* Couleur de texte courante */
void set_ct_cur(char ct){
	ct_cur=ct;
}

/* Couleur de fond courante */
void set_cf_cur(char cf){
	cf_cur=cf;
}

/* Colonne minimale */
void set_col_min(int limit){
  if((limit>=0)&&(limit<24)){
    col_min=limit;
  }
}

/* Pointeur memoire */
short *ptr_mem(int line, int col)
{
	return (short *)(0xB8000 + 2 * (line * NB_COLUMNS + col));
}



void write_char(int line, int col, char c, char ct, char cf, char cl)
{

  short *adr = ptr_mem(line, col);

  char formatting = (cl << 7) | (cf << 4) | (ct);

  *adr = c | formatting << 8;

}

void move_cursor(int line, int col)
{

  short pos = col + line * NB_COLUMNS;

  outb(0x0f, 0x3d4);
  outb(pos & 0xff, 0x3d5);
  outb(0x0e, 0x3d4);
  outb(pos >> 8, 0x3d5);

  line_cur = line;
  col_cur = col;
}

void initialize_screen()
{
  for (int i = 0; i < NB_LINES; i++) {
    for (int j = 0; j < NB_COLUMNS; j++) {
      write_char(i, j, ' ', ct_cur, cf_cur, 0);
    }
  }
  line_cur = first_line;
  col_cur = 0;
  col_min=0;
  move_cursor(line_cur, col_cur);
}

void clear_screen()
{
  for (int i = first_line; i < NB_LINES; i++) {
    for (int j = 0; j < NB_COLUMNS; j++) {
      write_char(i, j, ' ', ct_cur, cf_cur, 0);
    }
  }

  line_cur = first_line;
  col_cur = 0;
  move_cursor(line_cur, col_cur);
}

void scroll()
{
  if (line_cur > first_line) {
    //déplacement une ligne en haut
    memmove(ptr_mem(first_line, 0), ptr_mem(first_line+1, 0), NB_COLUMNS * (NB_LINES-first_line) * 2);
    line_cur--;
    //on ne doit pas atteindre la ligne où l'horloge est affichee ...
    if (line_cur == 0) 
      line_cur = first_line;

    move_cursor(line_cur, col_min);
  }
}

void handle_char(char c)
{
  if (c >= 32 && c <= 126) {
    //caractère imprimable
    if (line_cur < NB_LINES && col_cur < NB_COLUMNS) {
      write_char(line_cur, col_cur, c, ct_cur, cf_cur, 0);
      if (col_cur == NB_COLUMNS - 1) {
	col_cur = col_min;
	if (line_cur == NB_LINES - 1) {
	  scroll();
	}
	line_cur++;
      } else {
	col_cur++;
      }
    }
  } 
  else if(c == 127) {
    if (col_cur > col_min){
      col_cur=col_cur-1;
    }     
    write_char(line_cur, col_cur, ' ', ct_cur, cf_cur, 0);
  }
  else {
    //quelques caractères spéciaux
    switch (c) {
    case 8:
      write_char(line_cur, col_cur, ' ', ct_cur, cf_cur, 0);
      if (col_cur > col_min){
	col_cur=col_cur-1;
      }      	
      break;
			  
    case 9:
      col_cur = ((col_cur / 8) + 1) * 8;
      col_cur = col_cur < NB_COLUMNS ? col_cur : NB_COLUMNS - 1;	// on limite a la taille de l'écran
      break;

    case 10:
      if (line_cur == 24) {
	scroll();
      }
      line_cur++;
      col_cur = col_min;
      break;

    case 12:
      //Effacement de l'ecran de la console
      for (int j = col_min; j < NB_COLUMNS; j++) {
	write_char(first_line, j, ' ', ct_cur, cf_cur, 0);
      }
      for (int i = first_line+1; i < NB_LINES; i++) {
	for (int j = 0; j < NB_COLUMNS; j++) {
	  write_char(i, j, ' ', ct_cur, cf_cur, 0);
	}
      }

      line_cur = first_line;
      col_cur = col_min;
      move_cursor(line_cur, col_cur);
      break;

    case 13:
      if (line_cur == 24) {
	scroll();
      }
      line_cur++;
      col_cur = col_min;
      break;

    default:
      break;
    }
  }
  move_cursor(line_cur, col_cur);
}



