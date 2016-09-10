#include "stdio.h"
#include "string.h"
#include "syslib.h"
#include "cmd_lib.h"
#include "mem.h"
#include "shell.h"

// Calcul du minimum
#define MIN(x,y) ((x)<(y)?(x):(y))

static char line[LINE_SIZE+1];          // Ligne entree au clavier
static char** cmd_line=0;               // Sequence de mots composant la ligne
static int nb_words=4;                  // Nombre de mots par ligne

/*
 * Liberation de memoire
 */
static void free_cmd_line(void){
  char *cmd;
  
  for (int i=0; cmd_line[i]!=0; i++) {
    cmd = cmd_line[i]; 
    mem_free(cmd,strlen(cmd)+1);
  }   
  
  mem_free(cmd_line,nb_words*sizeof(char *));
}


/*
 * Parseur
 */
static void parse_cmd(void)
{ 
  int index=0;
  int incr=0;
  unsigned long nb_chars=0;
  char * start;    // Pointeurs de debut et fin de mot
  char cur; 

  // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  // @@@@@        SKIP_SPACE       @@@@@
  // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  while((cur=line[incr])==' '){
    incr++;
  }

  start=&line[incr];
  nb_chars++;
  incr++;
  
  if (start[0]=='\0'){
    cmd_line[0]=(char *)mem_alloc(sizeof(char));
    cmd_line[0][0]='\0';
  }
  else{    

    while(1){

      cur=line[incr];
      nb_chars++;
      incr++;

      if(cur=='\0'){
	unsigned long n = MIN(nb_chars, WORD_SIZE);
	cmd_line[index] = mem_alloc(n*sizeof(char));
	strncpy (cmd_line[index], start, n);
	cmd_line[index][n-1]='\0';
	return;
      }
      
      else if(cur==' '){
	unsigned long n =  MIN(nb_chars, WORD_SIZE);
	cmd_line[index]=(char *)mem_alloc(n*sizeof(char));
	strncpy (cmd_line[index], start, n);
	cmd_line[index][n-1]='\0';
	index++;

	// allocation tableau plus grand
	if (index==nb_words-1){
	  nb_words=nb_words*2;
	  char ** new_cmd_line= (char**) mem_alloc(nb_words*sizeof(char *));
	  for (int i=0; cmd_line[i]!=0; i++) {
	    new_cmd_line[i]=(char*) mem_alloc((strlen(cmd_line[i])+1)*sizeof(char));
	    strncpy (new_cmd_line[i], cmd_line[i], strlen(cmd_line[i]));
	  }
	  free_cmd_line();
	  cmd_line=new_cmd_line;
	}

	nb_chars=0;
	// SKIP_SPACE
	while((cur=line[incr])==' '){
	  incr++;
	}
	start=&line[incr];
	
      }
    }
  }
}




int start_shell(){
  
  while(1){
    // initialisation ligne de commande
    nb_words=4; 
    cmd_line=(char **)mem_alloc(nb_words*sizeof(char *));

    // prompt
    print_prompt(); 
  
    // Lecture et interpretation des commandes
    shell_cons_read(line,LINE_SIZE);
    
    // Parseur
    parse_cmd();
    
    if (strcmp(cmd_line[0],"echo")==0) {
      shell_echo();
    }
    else if (strcmp(cmd_line[0],"exit")==0) {
      shell_exit();
    }
    else if (strcmp(cmd_line[0],"ls")==0) {
      shell_ls();
    }
    else if (strcmp(cmd_line[0],"cd")==0) {
      shell_cd();
    }
    else if (strcmp(cmd_line[0],"ps")==0) {
      shell_ps();
    }
    else if (strcmp(cmd_line[0],"sinfo")==0) {
      shell_sinfo();
    }
    else if (strcmp(cmd_line[0],"clear")==0) {
      clear();
    }
    else if (strcmp(cmd_line[0],"")==0) {
    }
    else{        
      printf("\"%s\" : commande introuvable\n",cmd_line[0]);
    }
    
    // Liberation de memoire
    free_cmd_line();
  }
  return 0;
}
