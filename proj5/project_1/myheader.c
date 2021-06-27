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
char buf[1000];
void print(Node* cur, int connfd){
    if(cur==NULL)
        return;
    sprintf(buf,"%d %d %d\n",cur->ID,  cur->left_stock, cur->price);
    int len = strlen(buf);
    Rio_writen(connfd, buf, len);
    
    print(cur->left, connfd);
    print(cur->right, connfd);
}

int buy(Node* cur, int connfd, int buy_id, int buy_num){
    if(cur==NULL) return 0;
    if(cur->ID==buy_id){
        if(cur->left_stock<buy_num){ //남아있는 주식이 사려고 하는 주식보다 적을 경우
            sprintf(buf,"Not enough left stock\n");
        }
        else{
            sprintf(buf,"[buy] success\n");
            cur->left_stock -= buy_num;
        }
        Rio_writen(connfd, buf, strlen(buf));
        return 1;
    }
    if(cur->ID>buy_id){
        return buy(cur->left, connfd, buy_id, buy_num);
    }
    else
        return buy(cur->right, connfd, buy_id, buy_num);
}

int sell(Node* cur, int connfd, int sell_id, int sell_num){
    if(cur==NULL) return 0;
    if(cur->ID==sell_id){
        cur->left_stock += sell_num;
        sprintf(buf,"[sell] success\n");
        Rio_writen(connfd, buf, strlen(buf));
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

