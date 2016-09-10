extern int syscall(int call, int par1, int par2, int par3, int par4, int par5);

/*
 * Echo sur l'ecran
 */
void shell_echo(void){
  // Call n° 24
  syscall(24, 0, 0, 0, 0, 0);
}

/*
 * Sortie du noyau
 */
void shell_exit(void){
  // Call n° 25
  syscall(25, 0, 0, 0, 0, 0);
}

/*
 * Liste des processus
 */
void shell_ps(void){
  // Call n° 26
  syscall(26, 0, 0, 0, 0, 0);
}

/*
 * Liste des semaphores utilisees
 */
void shell_sinfo(void){
// Call n° 27
  syscall(27, 0, 0, 0, 0, 0);
}

/*
 * Fixation limite ecriture
 */
void set_col_min(int limit){
  // Call n° 28
  syscall(28, limit, 0, 0, 0, 0);
}


/*
 * Remise a zero du tampon
 */
int update_buffer(void){
  // Call n° 30
  return syscall(30, 0, 0, 0, 0, 0);
}

/*
 * Liste des fichiers et dossiers
 */
void shell_ls(void){
  // Call n° 31
  syscall(31, 0, 0, 0, 0, 0);
}

/*
 * Acces aux repertoires
 */
void shell_cd(void){
  // Call n° 32
  syscall(32, 0, 0, 0, 0, 0);
}


void print_prompt(void){
  // Call n° 33
  syscall(33, 0, 0, 0, 0, 0);
}
  
// Lecture et interpretation des commandes
void shell_cons_read(char * str,unsigned long length){
    // Call n° 34
  syscall(34, (int)str, (int)length, 0, 0, 0);
}

// Effacement ecran
void clear(void){
    // Call n° 35
  syscall(35, 0, 0, 0, 0, 0);
}
