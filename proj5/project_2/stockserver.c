/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"
#include "myheader.h"
#define SBUFSIZE 12
#define NTHREADS 12
sbuf_t sbuf;
void *thread(void* vargp){
    Pthread_detach(pthread_self());
    while(1){
        int connfd = sbuf_remove(&sbuf);
        
        echo_cnt(connfd);
        Close(connfd);
        if(sbuf.front==sbuf.rear){
            FILE* fp = fopen("stock.txt","w");
            print_txt(root,fp);
            fclose(fp);
        }
    }
}


int main(int argc, char **argv) 
{
    



    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    char client_hostname[MAXLINE], client_port[MAXLINE];
    pthread_t tid;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    
    FILE* fp = fopen("stock.txt","r");
    int current_id, current_price, reside_stock;
    while(1){
        int x = fscanf(fp,"%d %d %d", &current_id, &reside_stock, &current_price);
        if(x==EOF) break;
        root = push(root,current_id,reside_stock,current_price);
    }
    fclose(fp);
    int num_client = 0;

    listenfd = Open_listenfd(argv[1]);
    sbuf_init(&sbuf, SBUFSIZE);
    for(int i = 0;i<NTHREADS;i++){
        Pthread_create(&tid,NULL, thread, NULL);
    }
    while(1){

        /*여기 괜찮아 보이는데*/
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
                                client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        sbuf_insert(&sbuf,connfd);
    }
    exit(0);
}
/* $end echoserverimain */
