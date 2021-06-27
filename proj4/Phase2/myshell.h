/* $begin shellmain */
#include "csapp.h"
#include<errno.h>
#define MAXARGS   128

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 
int cd(char* path);
int parse_pipe(char** argv, char* argv_for_pipe[30][128]);
void pipe_line(char *argv_for_pipe[30][128], int argv_idx, int bg, char* cmdline);
