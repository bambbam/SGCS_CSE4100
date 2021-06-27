#include "myshell.h"

int main() 
{
    char cmdline[MAXLINE]; /* Command line */
    

    while (1) {
	/* Read */
    if(sigsetjmp(sigjmp,1)){
        Sio_puts("\n"); //sigint 가 들어왔을 때 엔터를 쳐준다.
    }
    Signal (SIGINT,  sigint_handler);
    Signal (SIGTSTP, SIG_IGN);
    Signal (SIGTTIN, SIG_IGN);
    Signal (SIGTTOU, SIG_IGN);
    Signal (SIGCHLD, sigchild_handler);
    main_pid = getpid();
    setpgid(main_pid, main_pid);
    tcsetpgrp(STDIN_FILENO, main_pid); //stdin을 하는 프로세스 그룹을 메인으로 세팅해둔다.
    tcgetattr(STDIN_FILENO,&main_terminal_attr);

    printf("CSE4100-SP-P4> ");                   
	fgets(cmdline, MAXLINE, stdin); 
	if (feof(stdin))
	    exit(0);

	/* Evaluate */
	eval(cmdline);
    } 
}
/* $end shellmain */


void pipe_line(char *argv_for_pipe[30][128], int argv_idx, int bg, char* cmdline){ //파이프라인을 처리하기 위한 함수이다.
    pid_t child_process[30];
    int idx = 0;
    int tempidx = 0;
    idx = 0;
    int pipes[30];
    
    sigset_t mask_all, mask_one, prev_one, mask_sigchld;
    Sigfillset(&mask_all);
    Sigemptyset(&mask_one);
    sigaddset(&mask_one, SIGCHLD); //차일드 시그널을 마스킹하는 핸들러를 하나 만든다.

    
    
    for(int i = 0;i<15;i++){
        pipe(pipes + 2*i);
    }
    //Sigprocmask(SIG_BLOCK, &mask_one, &prev_one);
    while(argv_for_pipe[idx][0] != NULL){
        if((child_process[idx] = fork())==0){
              
            pid_t pid = getpid();
            if(idx==0) {setpgid(pid,pid);child_process[0] = pid;}
            else setpgid(pid,child_process[0]);
            if(!bg){
                tcsetpgrp(STDIN_FILENO, child_process[0]); //프로세스 그룹을 지정해주고, 각각을 stdin의 기본으로 만들어준다.
            }
      //      Sigprocmask(SIG_SETMASK, &prev_one, NULL);
            Signal (SIGINT, SIG_DFL);   //시그널들을 디폴트 시그널로 풀어준다.
            Signal (SIGQUIT, SIG_DFL); //sigint or sigstop이 들어왔을 때 시그널은 default 행동인 프로세스를 죽이거나, 멈추는 행동을 할 것이다.
            Signal (SIGTSTP, SIG_DFL);
            Signal (SIGTTIN, SIG_DFL);
            Signal (SIGTTOU, SIG_DFL);
            
            
            int saved_stdout = dup(1); //커맨드가 없는 경우 사용해야하는 output이다
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
    Sigprocmask(SIG_BLOCK, &mask_all, NULL);
    addjob(child_process[0],bg, cmdline, argv_idx+1); //job을 하나로 묶어서 넣어준다. 프로세스 그룹의 main인 childprocess 0만을 넣어주었다.
    Sigprocmask(SIG_SETMASK, &prev_one, NULL);

    if (!bg){ 
        
        int child_status;
        //sleep(1);
        Sigprocmask(SIG_BLOCK, &mask_one, &prev_one);
        foreground_interrupt = 0; 
        while(!foreground_interrupt)
            sigsuspend(&prev_one); //시그널을 서스펜드한다. 만약 백그라운드가 죽어서 시그널이 들어올 수 있기 때문에, foreground_interrupt를 추가해주었다.
        //deletejob(pid);
        foreground_interrupt = 0;
        Sigprocmask(SIG_SETMASK, &prev_one, NULL);
        
    }
    else//when there is backgrount process!
        printf("%d %s", child_process[argv_idx-1], cmdline);

}


/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
    char original_cmdline[MAXARGS] = {0};
    strcpy(original_cmdline,cmdline);
    char *argv[MAXARGS] = {0}; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    
    sigset_t mask_all, mask_one, prev_one, mask_SIGSTOP;
    Sigfillset(&mask_all);
    Sigemptyset(&mask_one);
    sigaddset(&mask_one, SIGCHLD);

    
    strcpy(buf, cmdline);
    bg = parseline(buf, argv); 
    if (argv[0] == NULL)   
	return;   /* Ignore empty lines */ 
    char* argv_for_pipe[30][128]={0};
    


    int argv_idx = parse_pipe(argv,argv_for_pipe);
    if(argv_idx>0){
        pipe_line(argv_for_pipe,argv_idx,bg,cmdline);   
    }
    else if (!builtin_command(argv)) { 
        Sigprocmask(SIG_BLOCK, &mask_one, &prev_one);//quit -> exit(0), & -> ignore, other -> run
        if((pid=Fork()) == 0){
            pid = getpid();
            setpgid(pid,pid); //본인을 그룹의 장으로 하는 프로세스 그룹을 하나 만든다.
            if(!bg){
                tcsetpgrp(STDIN_FILENO, pid);
                //tcsetpgrp(STDOUT_FILENO, pid);   
            }
            Sigprocmask(SIG_SETMASK, &prev_one, NULL); 
            Signal (SIGQUIT, SIG_DFL);
            Signal (SIGTSTP, SIG_DFL);
            Signal (SIGTTIN, SIG_DFL);
            Signal (SIGTTOU, SIG_DFL);//시그널을 디폴트 시그널로 한다.
           // Signal (SIGCHLD, SIG_DFL);
            Signal (SIGINT, SIG_DFL);
            if (execvp(argv[0], argv) < 0) {	//ex) /bin/ls ls -al &
                char error_message[100];                
                sprintf(error_message,"%s: Command not found.\n", argv[0]);
                Sio_puts(error_message);
                exit(0);
            }
          
            exit(0);
        }


        

        /* Parent waits for foreground job to terminate */
        Sigprocmask(SIG_BLOCK, &mask_all, NULL);
        addjob(pid,bg, original_cmdline,1);
        Sigprocmask(SIG_SETMASK, &prev_one, NULL);


        if (!bg){ 
            int status;
            foreground_interrupt = 0;
            while(!foreground_interrupt)
                sigsuspend(&prev_one); //시그널을 foreground interrupt가 올때까지, 내가 원하는 시그널이 왔을때까지 기다린다.
            foreground_interrupt = 0;
            //sigsuspend();
            /*if(waitpid(pid,&status,WUNTRACED)<0){
                unix_error("waitfg: waitpid error");
            }*/
        }
        else//when there is baSIG_DFLckgrount process!
            printf("%d %s", pid, cmdline);
    }
    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    if (!strcmp(argv[0], "exit")){ /* quit command */
	    for(int i = 0;i<MAXJOBS;i++){
            if(jobs[i].used){
                kill(-jobs[i].pid,SIGKILL);
            }
        }
        exit(0);  
    }
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
    if(!strcmp(argv[0],"kill")){
        if(argv[1]==0){
            printf("too few argument\n");
        }
        else if(argv[2]!=0){
            printf("Too Many Argument\n");
        }
        else{
            int to_kill = atoi(argv[1]+1);
            for(int i = 0;i<MAXJOBS;i++){
                if(jobs[i].used){
                    if(jobs[i].jid==to_kill){
                        kill(-(jobs[i].pid),SIGKILL);
                        deletejob(jobs[i].pid);
                        break;
                    }
                }    
            }
        }
        return 1;
    }
    if(!strcmp(argv[0],"jobs")){
        if(argv[1]!=0){
            printf("Too Many Argument\n");
        }
        else print_jobs();
        return 1;
    }
    if(!strcmp(argv[0],"bg")){
        if(argv[2]!=0){
            printf("Too Many Argument\n");
        }
        else if(argv[1]==0){
            printf("Too few Argument\n");
        }
        else{
            int to_kill = atoi(argv[1]+1);
            bg(to_kill);
        }
        return 1;
    }
    if(!strcmp(argv[0],"fg")){
        if(argv[2]!=0){
            printf("Too Many Argument\n");
        }
        else if(argv[1]==0){
            printf("Too few Argument\n");
        }
        else{
            int to_kill = atoi(argv[1]+1);
            fg(to_kill);
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

int parse_pipe(char** argv, char* argv_for_pipe[30][128]) {
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


int cd(char* path){
    char current[100];
    if(path==NULL){
        char* homedir = getenv("HOME");
        return chdir(homedir);
    }
    else if(path[0]=='~'){
        char* homedir = getenv("HOME"); //nothing is entered
        return chdir(homedir);
    }
    else if(path[0]!='/'){
        getcwd(current,sizeof(current));
        strcat(current,"/");
        strcat(current,path);
        return chdir(current);
    }
    return chdir(path);
}

void addjob(pid_t pid, int bg, char* cmdline, int jobnum){ //job을 하나 추가한다.
    for(int i = 0;i<MAXJOBS;i++){
        if(jobs[i].used == 0){
            jobs[i].pid = pid;
            jobs[i].status = bg;
            jobs[i].jid = JID++; 
            strcpy(jobs[i].args,cmdline);
            jobs[i].used = 1;
            jobs[i].job_num = jobnum;
            if(JID>MAXJOBS) JID = 1;
            return;
        }
    }
}

int deletejob(pid_t pid){ //job을 하나 뺀다.
    for(int i = 0;i<MAXJOBS;i++){
        if(jobs[i].used&&jobs[i].pid==pid){
            int ret = (jobs[i].status != 0);
            jobs[i].pid = 0;
            jobs[i].status = 0;
            jobs[i].jid = 0; 
            for(int j = 0;j<100;j++) jobs[i].args[j] = 0;
            jobs[i].used = 0;
            JID = 0;
            for(int j = 0;j<MAXJOBS;j++){
                if(jobs[j].used!=0){
                    if(JID<jobs[j].jid) JID = jobs[j].jid;
                }
            }
            JID = JID + 1;
            return ret;
        }
    }
    return 0;
}

void print_jobs(){
    for(int i = 0;i<MAXJOBS;i++){
        char buf[100];
        if(jobs[i].used){
            Sio_puts("[");
            Sio_putl(jobs[i].jid);
            
            if(jobs[i].status==0) {
                Sio_puts("]   Foreground\t\t\t");
                Sio_puts(jobs[i].args);
                //sprintf(buf,"[%d]   Foreground\t\t\t%s",jobs[i].jid, jobs[i].args);   
            }
            if(jobs[i].status==1){
                Sio_puts("]   Background\t\t\t");
                Sio_puts(jobs[i].args);
                // sprintf(buf,"[%d]   Background\t\t\t%s",jobs[i].jid, jobs[i].args);
            }
            if(jobs[i].status==2) {
                Sio_puts("]      Stopped\t\t\t");
                Sio_puts(jobs[i].args);
                //sprintf(buf,"[%d]      Stopped\t\t\t%s",jobs[i].jid, jobs[i].args);   
            }
        }
        
        
    }
}

void sigint_handler(){
    siglongjmp(sigjmp,1);
}

void sigchild_handler(){
    int status;
    pid_t pid;
    int x ;
    while((pid=waitpid(-1,&status, WNOHANG|WUNTRACED)) > 0){ 
        if(WIFEXITED(status)){ //정상적으로 죽었을 경우
            for(int i = 0;i<MAXJOBS;i++){
                if(jobs[i].used&&jobs[i].pid==pid){
                    jobs[i].job_num--;
                    if(jobs[i].job_num>0){
                        int iter = jobs[i].job_num;
                        for(int i = 0;i<iter;i++){
                            waitpid(-(pid),&status,WUNTRACED); //파이프라인인 경우 파이프라인만큼 더 기다려준다.
                            if(!WIFSTOPPED(status)){
                                jobs[i].job_num--;
                            }
                        }
                        break;
                    }
                    else break;
                }
            }
            if(WIFEXITED(status)){ //다 기다렸음에도 정상적으로 죽은 경우
                if(deletejob(pid)){ //백그라운드 인 경우 메세지를 출력한다.
                    Sio_puts("\n[");
                    Sio_putl(pid);
                    Sio_puts("]");
                    Sio_puts("is finished\n");
                }
                else{
                    foreground_interrupt = 1; //포그라운드인 경우 포그라운드 인터럽트를 set한다.
                }
            }
            
        }
        if(WIFSIGNALED(status)){    //비정상적으로 끝난 경우, 여기서는 컨트롤 c가 그것에 해당한다.
            for(int i = 0;i<MAXJOBS;i++){
                if(jobs[i].used&&jobs[i].pid==pid){ 
                    Sio_putl(pid);
                    Sio_putl(jobs[i].jid);
                    Sio_puts("[");
                    Sio_putl(jobs[i].jid);
                    Sio_puts("] is interruptted\n");
                    deletejob(pid);
                    foreground_interrupt = 1;
                    break;
                }                
            }
        }
        if(WIFSTOPPED(status)){ //sigstop signal을 받은 경우
            for(int i = 0;i<MAXJOBS;i++){
                if(jobs[i].used&&jobs[i].pid==pid){
                    Sio_puts("[");
                    Sio_putl(jobs[i].jid);
                    Sio_puts("] is suspended\n");
                    jobs[i].status = 2;
                    foreground_interrupt = 1;
                    break;
                }
            }
        }
        if(foreground_interrupt){ //foreground 인터럽트를 받은 경우 터미널의 stdin 제어권을 메인 프로세스로 넘겨준다.
            tcsetpgrp(STDIN_FILENO, main_pid);
            tcsetattr(STDIN_FILENO, TCSADRAIN, &main_terminal_attr);
        }
    }
}


void bg(int jid){ //백그라운드 명령이 들어온 경우
    for(int i = 0;i<MAXJOBS;i++){
        if(jobs[i].used&&jobs[i].jid==jid){
             if(kill(-getpgid(jobs[i].pid),SIGCONT)){
                 Sio_puts("this process doesn't continued\n");
                 return;
             }
             jobs[i].status = 1;
             return;
        }
    }   
    Sio_puts("there is no JID like that\n");
}


void fg(int jid){ //포그라운드 명령이 들어온 경우
    for(int i = 0;i<MAXJOBS;i++){
        if(jobs[i].used&&jobs[i].jid==jid){
            tcsetpgrp(STDIN_FILENO, (jobs[i].pid));
            jobs[i].status = 0;
            kill(-(jobs[i].pid),SIGCONT);  //컨티뉴를 하고
            int status;
            sigset_t emtpy;
            waitpid(-(jobs[i].pid),&status,WUNTRACED); //그것을 기다린다.
            pid_t pid = jobs[i].pid;
            if(WIFEXITED(status)){        //정상적으로 종료가 되었다면
                for(int i = 0;i<MAXJOBS;i++){
                    if(jobs[i].used&&jobs[i].pid==pid){
                        jobs[i].job_num--;
                        if(jobs[i].job_num>0){
                            int iter = jobs[i].job_num;
                            for(int i = 0;i<iter;i++){
                                waitpid(-1,&status,WUNTRACED);
                                if(!WIFSTOPPED(status)){
                                    jobs[i].job_num--;
                                }
                            }
                            break;
                        }
                        else break;
                    }
                }
                if(WIFEXITED(status)&&deletejob(pid)){ //백그라운드라면 메세지를 출력한다.
                    Sio_puts("\n[");
                    Sio_putl(pid);
                    Sio_puts("]");
                    Sio_puts("is finished\n");
                }
            }
            if(WIFSIGNALED(status)){     //비정상적으로 시그널을 뱉으며 죽은경우
                for(int i = 0;i<MAXJOBS;i++){
                    if(jobs[i].used&&jobs[i].pid==pid){
                        Sio_putl(pid);
                        Sio_putl(jobs[i].jid);
                        Sio_puts("[");
                        Sio_putl(jobs[i].jid);
                        Sio_puts("] is interruptted\n");
                        deletejob(pid);
                        break;
                    }                
                }
            } 
            if(WIFSTOPPED(status)){//멈춘 경우
                for(int i = 0;i<MAXJOBS;i++){
                    if(jobs[i].used&&jobs[i].pid==pid){
                        Sio_puts("[");
                        Sio_putl(jobs[i].jid);
                        Sio_puts("] is suspended\n");//printf("[%d] is killed\n", jobs[i].jid);
                        jobs[i].status = 2;
                        break;
                    }
                }
            }
            tcsetpgrp(STDIN_FILENO, main_pid);
            tcsetattr(STDIN_FILENO, TCSADRAIN, &main_terminal_attr);
            return;
        }
    }
    Sio_puts("there is no JID like that\n");
}
