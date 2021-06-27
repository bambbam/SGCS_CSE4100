#include "myheader.h"

void clear_tree(Node* root){
    if(root==NULL) return;
    clear_tree(root->left);
    clear_tree(root->right);
    free(root);
}

Node* push(Node* cur, int id, int left_stock, int price){
    if(cur==NULL){
        Node* ret = (Node*)calloc(sizeof(Node),1);
        ret->ID = id;
        ret->price = price; 
        ret->left_stock = left_stock;
        Sem_init(&(ret->mutex_r), 0, 1);
        Sem_init(&(ret->mutex_w), 0, 1);
        ret->readcnt = 0;
        ret->left = NULL;
        ret->right = NULL;
        return ret;
    }
    if(id<cur->ID){
        cur->left = push(cur->left, id, left_stock, price);
    }
    else{
        cur->right = push(cur->right, id, left_stock, price);
    }
    return cur;
}

void push_back(char* real_buf, char* back){
    int len = strlen(real_buf);
    int len2 = strlen(back);
    for(int i = len;i<len+len2;i++){
        real_buf[i] = back[i-len];
    }
    real_buf[len+len2] = 0;
}


void print(Node* cur, int connfd, char* real_buf){
    if(cur==NULL)
        return;

    char buf[100] = {0};
    P(&(cur->mutex_r));
    cur->readcnt++;
    if(cur->readcnt==1){
        P(&(cur->mutex_w));
    }
    V(&(cur->mutex_r));

    sprintf(buf,"%d %d %d\n",cur->ID,  cur->left_stock, cur->price);
    int len = strlen(buf);
    //Rio_writen(connfd, buf, len);
    push_back(real_buf, buf);
    P(&(cur->mutex_r));
    cur->readcnt--;
    if(cur->readcnt==0){
        V(&(cur->mutex_w));
    }
    V(&(cur->mutex_r));

    print(cur->left, connfd, real_buf);
    print(cur->right, connfd, real_buf);
}

int buy(Node* cur, int connfd, int buy_id, int buy_num){
    char buf[100]= {0};
    if(cur==NULL) return 0;
    if(cur->ID==buy_id){
        P(&(cur->mutex_w));
        if((cur->left_stock)<buy_num){ //남아있는 주식이 사려고 하는 주식보다 적을 경우
            sprintf(buf,"Not enough left stock\n");
        }
        else{
            sprintf(buf,"[buy] success\n");
            cur->left_stock -= buy_num;
        }
        Rio_writen(connfd, buf, strlen(buf));
        V(&(cur->mutex_w));
        return 1;
    }
    if(cur->ID>buy_id){ 
        return buy(cur->left, connfd, buy_id, buy_num);
    }
    else
        return buy(cur->right, connfd, buy_id, buy_num);
}

int sell(Node* cur, int connfd, int sell_id, int sell_num){
    char buf[100]= {0};
    if(cur==NULL) return 0;
    if(cur->ID==sell_id){
        P(&(cur->mutex_w));
        cur->left_stock += sell_num;
        sprintf(buf,"[sell] success\n");
        Rio_writen(connfd, buf, strlen(buf));
        V(&(cur->mutex_w));
        return 1;
    }
    if(cur->ID>sell_id){
        return sell(cur->left, connfd, sell_id, sell_num);
    }
    else
        return sell(cur->right, connfd, sell_id, sell_num);
}


void process_char(char* buf, char** ret){
    char* ptr = strtok(buf," ");
    int i = 0;
    while(ptr!=NULL){
        ret[i++] = ptr; 
        ptr = strtok(NULL," ");
    }
    ret[i] = "";
}

void print_txt(Node* root, FILE* fp){
    if(root==NULL) return;
    fprintf(fp,"%d %d %d\n",root->ID, root->left_stock, root->price);
    print_txt(root->left,fp);
    print_txt(root->right,fp);
}

/* Create an empty, bounded, shared FIFO buffer with n slots */
void sbuf_init(sbuf_t *sp, int n)
{
    sp->buf = Calloc(n, sizeof(int));
    sp->n = n; /* Buffer holds max of n items */
    sp->front = sp->rear = 0; /* Empty buffer iff front == rear */
    Sem_init(&sp->mutex, 0, 1); /* Binary semaphore for locking */
    Sem_init(&sp->slots, 0, n); /* Initially, buf has n empty slots */
    Sem_init(&sp->items, 0, 0); /* Initially, buf has 0 items */
}
/* Clean up buffer sp */
void sbuf_deinit(sbuf_t *sp)
{
    Free(sp->buf);
}
/* Insert item onto the rear of shared buffer sp */
void sbuf_insert(sbuf_t *sp, int item)
{
    P(&sp->slots); /* Wait for available slot */
    P(&sp->mutex); /* Lock the buffer */
    sp->buf[(++sp->rear)%(sp->n)] = item; /* Insert the item */
    V(&sp->mutex); /* Unlock the buffer */
    V(&sp->items); /* Announce available item */
}
/* Remove and return the first item from buffer sp */
int sbuf_remove(sbuf_t *sp)
{
    int item;
    P(&sp->items); /* Wait for available item */
    P(&sp->mutex); /* Lock the buffer */
    item = sp->buf[(++sp->front)%(sp->n)]; /* Remove the item */
    V(&sp->mutex); /* Unlock the buffer */
    V(&sp->slots); /* Announce available slot */
    return item;
}