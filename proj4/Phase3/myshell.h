/* $begin shellmain */
#include "csapp.h"
#include<errno.h>
#include <termios.h>
#define MAXARGS   128
#define MAXJOBS   16
/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 
int cd(char* path);
int parse_pipe(char** argv, char* argv_for_pipe[30][128]);


/*for jobs*/
typedef struct {
    pid_t pid;
    int jid;
    int status;
    char args[100];
    int used;
    int job_num;
} job_type;
int JID = 1;
job_type jobs[MAXJOBS];
int joblen = 0;
void init_job();
void addjob(pid_t pid, int bg, char* cmdline, int jobnum);
int deletejob(pid_t pid);
void print_jobs();

void sigchild_handler();
void sigint_handler();
void sigstop_handler();


void sigint_handler_for_child();
void sigstop_handler_for_child();
int main_pid;
void bg(int);
void fg(int);

sigjmp_buf sigjmp;
struct termios main_terminal_attr;

int foreground_interrupt;