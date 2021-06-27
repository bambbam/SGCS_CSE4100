#include "opcode.h"
/**********************************
함수 : hash(const char* code)
목적 : code에 들어있는 값을 모두 더해서 20으로 나눈 나머지를 구함으로써 값들을 분배해준다.
return : code에 들어있는 값을 모두 더해서 20으로 나눈 나머지
************************************/
int hash(const char* code){
    int len = strlen(code);
    int ret = 0;
    for(int i = 0;i<len;i++){
        ret += code[i];
    }
    return ret%20;
}
/**********************************
함수 : init_hash_table()
목적 : opcode.txt에 들어있는 해시테이블을 모두 해싱해서
헤시테이블에 링크드 리스트 형태로 넣어준다.
return : 파일이 열리지 않으면 false
파일이 열렸으면 true
************************************/
bool init_hash_table(){
    FILE* fp = fopen("opcode.txt","r");
    if(fp==NULL){
        printf("file open failure");
        return false;
    }
    for (int i = 0; i < 20; i++) {
        hashTable[i] = (hashNode*)calloc(1,sizeof(hashNode));
    }
    int code;
    char format[100];
    char mnemonic[100];
    while(fscanf(fp,"%X %s %s",&code,mnemonic,format)!=EOF){
        opcode* cur_opcode = (opcode*)calloc(1,sizeof(opcode));
        cur_opcode->code = code;
        cur_opcode->next = NULL;
        strcpy(cur_opcode->format,format);
        strcpy(cur_opcode->mnemoinc,mnemonic);
        push_to_hash(cur_opcode);
    }
    fclose(fp);
    return true;
}
/**********************************
함수 : push_to_hash(opcode* cur_opcode)
목적 : 해시테이블의 해당하는 row의 링크드 리스트에 제일 마지막 원소에 이번 cur_opcode를 넣어준다.
return : 없음
************************************/
void push_to_hash(opcode* cur_opcode){
    int idx = hash(cur_opcode->mnemoinc);
    if(hashTable[idx]->head==NULL){
        hashTable[idx]->head = cur_opcode;
        hashTable[idx]->tail = cur_opcode;
    }
    else{
        hashTable[idx]->tail->next = cur_opcode;
        hashTable[idx]->tail = cur_opcode;
    }
}
/**********************************
함수 : opcodelist()
목적 : init_hash_table에서 구성한 hashtable을 hash값에 맞춰서 출력을 해준다.
return : 없음
************************************/
void opcodelist(){
    for(int i = 0;i<20;i++){
        opcode* iter = hashTable[i]->head;
        printf("%d : ",i);
        if(iter!=NULL){
            while(iter->next!=NULL){
                printf("[%s,%02X] -> ",iter->mnemoinc,iter->code);
                iter = iter->next;
            }  
            printf("[%s,%02X]",iter->mnemoinc,iter->code);
        }
        printf("\n");
    }
}
/**********************************
함수 : mnemonic(const char* tofind)
목적 : 입력받은 mnemonic(tofind)에 맞는 code값을 찾아준다.
return : 없음
************************************/
bool mnemonic(const char* tofind){
    int idx = hash(tofind);
    opcode* iter = hashTable[idx]->head;
    while(iter!=NULL){
        if(!strcmp(tofind,iter->mnemoinc)){
            printf("opcode is %02X\n",iter->code);
            return true;
        }
        iter = iter->next;
    }
    return false;
}

/**********************************
함수 : free_hash_table()
목적 : 프로그램이 끝났을 떄 동적할당 받았던 해시 테이블을 할당해제해준다.
return : 없음
************************************/
void free_hash_table(){
    for(int i = 0;i<20;i++){
        opcode* iter = hashTable[i]->head;
        while(iter!=NULL){
            opcode* tofree = iter;
            iter = iter->next;
            free(tofree);
        }
        free(hashTable[i]);
    }
}




//2에서 추가되었음
int get_code(const char* tofind){
    int idx = hash(tofind);
    opcode* iter = hashTable[idx]->head;
    while(iter!=NULL){
        if(!strcmp(tofind,iter->mnemoinc)){
            return iter->code;
        }
        iter = iter->next;
    }
    return -1;
}

char* get_format(const char* tofind){
    int idx = hash(tofind);
    opcode* iter = hashTable[idx]->head;
    while(iter!=NULL){
        if(!strcmp(tofind,iter->mnemoinc)){
            return iter->format;
        }
        iter = iter->next;
    }
    return NULL;
}


char* opcode_to_format(int code){
    for(int i = 0;i<20;i++){
        opcode* iter = hashTable[i]->head;
        while(iter!=NULL){
            if(iter->code==code){
                return iter->format;
            }
            iter = iter->next;
        }
    }
    return NULL;
}
