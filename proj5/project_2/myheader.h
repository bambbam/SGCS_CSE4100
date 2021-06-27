#include "csapp.h"

typedef struct {
    int *buf; /* Buffer array */
    int n; /* Maximum number of slots */
    int front; /* buf[(front+1)%n] is first item */
    int rear; /* buf[rear%n] is last item */
    sem_t mutex; /* Protects accesses to buf */
    sem_t slots; /* Counts available slots */
    sem_t items; /* Counts available items */
} sbuf_t;
void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp); 

typedef struct BINARY_TREE{
    int ID;
    int left_stock;
    int price;
    int readcnt;
    sem_t mutex_r;
    sem_t mutex_w;
    struct BINARY_TREE* left, *right;
}Node;
Node* root;
void clear_tree(Node* root);
Node* push(Node* cur, int id, int left_stock, int price);
void print(Node* cur, int connfd, char*);
int buy(Node* cur, int connfd, int buy_id, int buy_num);
int sell(Node* cur, int connfd, int sell_id, int sell_num);
void process_char(char* buf, char** ret);
void print_txt(Node* root, FILE* fp);

