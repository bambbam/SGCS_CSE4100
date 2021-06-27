/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"
#include "myheader.h"

int main(int argc, char **argv) 
{

    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    
    fd_set master; 
    fd_set read_fds; 
    int fdmax; 




    listenfd = Open_listenfd(argv[1]);


    FD_SET(listenfd, &master); 
    fdmax = listenfd; 


    FILE* fp = fopen("stock.txt","r");
    int current_id, current_price, reside_stock;
    while(1){
        int x = fscanf(fp,"%d %d %d", &current_id, &reside_stock, &current_price);
        if(x==EOF) break;
        root = push(root,current_id,reside_stock,current_price);
    }
    fclose(fp);
    int num_client = 0;
    while (1) {
        read_fds = master;
        if(Select(fdmax+1, &read_fds, NULL,NULL,NULL) == -1){
            perror("select");
            exit(1);   
        }
        
        for(int i= 0;i<=fdmax;i++){
            if(FD_ISSET(i,&read_fds)){
                if(i==listenfd){
                    clientlen = sizeof(struct sockaddr_storage); 
                    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
                    if(connfd==-1){
                        perror("accept");
                    }
                    else{
                        FD_SET(connfd, &master);
                        if(connfd > fdmax){
                            fdmax = connfd;
                        }
                        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
                                client_port, MAXLINE, 0);
                        printf("Connected to (%s, %s)\n", client_hostname, client_port);
                        num_client++;
                    }
                }
                else{    
                    if(echo(i)==0){
                        Close(i);
                        FD_CLR(i,&master);
                        num_client--;
                    }
                }
            }    
        }
        if(num_client==0){
            FILE* fp = fopen("stock.txt","w");
            print_txt(root,fp);
            fclose(fp);
        }
    }
    exit(0);
}
/* $end echoserverimain */
