/*
 * echo - read and echo text lines until client closes connection
 */
/* $begin echo */
#include "csapp.h"
#include "myheader.h"


int echo(int connfd) 
{
    int n; 
    char buf[MAXLINE]; 
    char* command[15];
    rio_t rio;
    

    Rio_readinitb(&rio, connfd);
    //while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        n = Rio_readlineb(&rio, buf, MAXLINE);
        if(n==0) {
            return 0;
        }
        printf("server received %d bytes\n", n);
        if(buf[n-1]=='\n') buf[--n] = '\0';
        process_char(buf,command);
        if(!strcmp("show",command[0])){
            print(root,connfd);    
        }
        else if(!strcmp("buy",command[0])){
            buy(root,connfd, atoi(command[1]),atoi(command[2]));
        }
        else if(!strcmp("sell",command[0])){
            sell(root,connfd, atoi(command[1]),atoi(command[2]));
        }

        Rio_writen(connfd,"\n",1);
    
    return n+1;
}
/* $end echo */

