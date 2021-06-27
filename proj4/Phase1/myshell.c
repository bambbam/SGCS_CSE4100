#include "myshell.h"

int main() 
{
    char cmdline[MAXLINE]; /* Command line */

    while (1) {
	/* Read */
	printf("CSE4100-SP-P4> ");                   
	fgets(cmdline, MAXLINE, stdin); 
	if (feof(stdin))
	    exit(0);

	/* Evaluate */
	eval(cmdline);
    } 
}
/* $end shellmain */


void pipe_line(char *argv_for_pipe[30][128], int argv_idx, int bg, char* cmdline){ //파이프라인을 수행하기 위한 함수이다. for phase 2
    pid_t child_process[30];
    int idx = 0;
    int tempidx = 0;
    idx = 0;
    int pipes[30];
    for(int i = 0;i<15;i++){
        pipe(pipes + 2*i);
    }
    
    while(argv_for_pipe[idx][0] != NULL){
        if((child_process[idx] = fork())==0){
            int saved_stdout = dup(1);
            if(idx!=0){
                dup2(pipes[2*(idx-1)],0);
            }
            if(idx != argv_idx){
                dup2(pipes[idx*2+1],1);
            }
            for(int i = 0;i<30;i++){
                close(pipes[i]);
            }
            if(execvp(argv_for_pipe[idx][0],argv_for_pipe[idx])<0){
                char error_message[100];
                dup2(saved_stdout,1);
                close(saved_stdout);
                sprintf(error_message,"%s: Command not found.\n", argv_for_pipe[idx][0]);
                Sio_puts(error_message);
                exit(0);
            }
        
            exit(0);
        }
        idx++;
    }

    for(int i = 0;i<30;i++){
        close(pipes[i]);
    }
    if (!bg){ 
        int child_status;
        for(int i= 0;i<=argv_idx;i++){
            Waitpid(child_process[i],&child_status,0);
        }
    }
    else//when there is backgrount process!
        printf("%d %s", child_process[argv_idx-1], cmdline);
}


/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
    char *argv[MAXARGS] = {0}; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    
    strcpy(buf, cmdline);
    bg = parseline(buf, argv); 
    if (argv[0] == NULL)   
	return;   /* Ignore empty lines */ 
    char* argv_for_pipe[30][128]={0};
    
    int argv_idx = parse_pipe(argv,argv_for_pipe);
    if(argv_idx>0){
        printf("Phase 1 can't use pipe\n");    
        return;
    }
    else if (!builtin_command(argv)) { //quit -> exit(0), & -> ignore, other -> run
        if((pid=Fork()) == 0){ //child process fork
            if (execvp(argv[0], argv) < 0) {	//ex) /bin/ls ls -al &
                char error_message[100];                
                sprintf(error_message,"%s: Command not found.\n", argv[0]);
                Sio_puts(error_message);
                exit(0);
            }
            exit(0);
        }
        /* Parent waits for foreground job to terminate */
        if (!bg){ 
            int status;
            if(waitpid(pid,&status,0)<0){ //child process reap
                unix_error("waitfg: waitpid error");
            }
        }
        else//when there is backgrount process!
            printf("%d %s", pid, cmdline); //background process
    }
    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    if (!strcmp(argv[0], "exit")) /* quit command */
	exit(0);  
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
	return 1;
    if(!strcmp(argv[0],"cd")){
        if(argv[2]!=0){
            printf("%s: Too many arguments\n",argv[0]);
        }
        else{
            if(cd(argv[1])<0){
                printf("%s: %s : No such file or directory\n",argv[0], argv[1]);
            }
        }
        return 1;            
    }
    return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	    buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
	return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	    argv[--argc] = NULL;
    int templine = strlen(argv[argc-1]);
    if((argv[argc-1][templine-1]=='&')){
        argv[argc-1][--templine] = NULL;
        bg = 1;
    }
    
    return bg;
}
/* $end parseline */

int parse_pipe(char** argv, char* argv_for_pipe[30][128]) { //파이프라인에 들어갈 명령어를 파싱하기 위한 함수이다. for phase2
    int idx = 0;
    int pipe_idx = 0;
    int num_pipe = 0;
    char* to_insert = argv[idx];
    while (argv[idx] != NULL) {
        char* cur = argv[idx];
        to_insert = cur;
        int t = 0;


        if (!strcmp(cur, "|")) {
            pipe_idx++;
            num_pipe = 0;
            idx++;
            continue;
        }
        while (*cur != '\0') {
            if (*cur == '|') {
                *cur = '\0';
                if (t != 0) {
                    argv_for_pipe[pipe_idx][num_pipe++] = to_insert;
                }
                to_insert = cur + 1;
                pipe_idx++;
                num_pipe = 0;
                
            }
            cur++;
            t++;
        }
        if(*(cur-1)) argv_for_pipe[pipe_idx][num_pipe++] = to_insert;
        idx++;
    }
    
    return pipe_idx;
}


int cd(char* path){ //change directory
    char current[100];
    if(path==NULL){
        char* homedir = getenv("HOME"); //nothing is entered
        return chdir(homedir); // go to home directory
    }
    else if(path[0]=='~'){
        char* homedir = getenv("HOME"); //nothing is entered
        return chdir(homedir);
    }
    else if(path[0]!='/'){
        getcwd(current,sizeof(current)); //current directory
        strcat(current,"/"); 
        strcat(current,path); //crrent directory + entered path
        return chdir(current);
    }
    return chdir(path); //절대경로
}
