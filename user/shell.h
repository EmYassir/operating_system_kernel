#ifndef __SHELL_H__
#define __SHELL_H__

#define LINE_SIZE 100
#define WORD_SIZE  50


/* Structure returned by readcmd() */
struct cmd_line {
  char ***seq;	// Sequence d'instructions 
};



int start_shell();

#endif
