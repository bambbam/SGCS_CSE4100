#include "csapp.h"


typedef struct BINARY_TREE{
    int ID;
    int left_stock;
    int price;
    //int readcnt;
    struct BINARY_TREE* left, *right;
}Node;
Node* root;
void clear_tree(Node* root);
Node* push(Node* cur, int id, int left_stock, int price);
void print(Node* cur, int connfd);
int buy(Node* cur, int connfd, int buy_id, int buy_num);
int sell(Node* cur, int connfd, int sell_id, int sell_num);
void process_char(char* buf, char** ret);
void print_txt(Node* root, FILE* fp);