#ifndef __CMD_LIB_H__
#define __CMD_LIB_H__


/******************************************************************************/
/*                          Interprete de commandes                           */
/******************************************************************************/

/*
 * Echo sur l'ecran
 */
void shell_echo(void);

/*
 * Sortie du noyau
 */
void shell_exit(void);

/*
 * Liste des fichiers et dossiers
 */
void shell_ls(void);

/*
 * Liste des processus
 */
void shell_ps(void);

/*
 * Acces a un repertoire
 */
void shell_cd(void);

/*
 * Liste des semaphores utilisees
 */
void shell_sinfo(void);

/*
 * Fixation limite ecriture
 */
void set_col_min(int limit);


/*
 * Remise a zero du tampon
 */
int update_buffer(void);

/*
 * Prompt a l'ecran
 */
void print_prompt(void);

/*
 * cons_read du clavier
 */
void shell_cons_read(char * str,unsigned long length);

/*
 * effacement de l'ecran
 */
void clear(void);
#endif
