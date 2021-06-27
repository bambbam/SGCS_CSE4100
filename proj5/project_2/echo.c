/*
 * echo - read and echo text lines until client closes connection
 */
/* $begin echo */
#include "csapp.h"
#include "myheader.h"


/* $end echo */
static int byte_cnt;
static void init_echo_cnt(void){
    byte_cnt = 0;
}
void echo_cnt(int connfd){
    int n;
    char buf[MAXLINE];
    char* command[15];
    char real_buf[5000] = {0};
    rio_t rio;
    //static pthread_once_t once = PTHREAD_ONCE_INIT;
    //pthread_once(&once, init_echo_cnt);
    Rio_readinitb(&rio, connfd);
    while((n=Rio_readlineb(&rio, buf, MAXLINE)) != 0){
        printf("server received %d bytes\n", n);
        if(buf[n-1]=='\n') buf[--n] = '\0';
        process_char(buf,command);
        if(!strcmp("show",command[0])){
            print(root,connfd, real_buf);    
            Rio_writen(connfd, real_buf, strlen(real_buf));
            //Rio_writen(connfd, buf, len);
        }
        else if(!strcmp("buy",command[0])){
            buy(root,connfd, atoi(command[1]),atoi(command[2]));
        }
        else if(!strcmp("sell",command[0])){
            sell(root,connfd, atoi(command[1]),atoi(command[2]));
        }
        Rio_writen(connfd,"\n",1);
        memset(real_buf,0,sizeof(real_buf));
    }
    
}