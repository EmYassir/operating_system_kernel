#ifndef __CONSOLE_H__
#define __CONSOLE_H__

/* Dimensions de la console */
#define NB_LINES	25
#define NB_COLUMNS	80
#define FIRST_LINE	 1




/*
 * Accesseurs
 */

/* Ligne courante */
int get_line_cur();

/* Colonne courante */
int get_col_cur();

/* Couleur de texte courante */
char get_ct_cur();

/* Couleur de fond courante */
char get_cf_cur();

/* Colonne minimale */
int get_col_min();

/*
 * Mutateurs
 */

/* Ligne courante */
void set_line_cur(int line);

/* Colonne courante */
void set_col_cur(int col);

/* Couleur de texte courante */
void set_ct_cur(char ct);

/* Couleur de fond courante */
void set_cf_cur(char cf);

/* Colonne minimale */
void set_col_min(int limit);

short *ptr_mem(int line, int col);

void write_char(int line, int col, char c, char ct, char cf, char cl);

void move_cursor(int line, int col);

void initialize_screen();

void clear_screen();

void handle_char(char c);

void scroll();

extern void console_putbytes(const char *s, int len);

#endif
