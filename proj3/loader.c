#include "loader.h"
#include "memory.h"

ET ESTAB; //external symbol table
int convert_signed_hex(char* num){ //음수의 2's complement로 표현된 숫자를 1's complement로 바꾼다.
    int x = convert_hex_to_int(num);
    if(x>=0x800000){
        x^=0xFFFFFF;
        x+=1;
        x*=-1;
    }
    return x;
}


int set_progaddr(char* address){ //progaddr를 address로 바꿔준다.
    int addr = convert_hex_to_int(address);
    if(is_valid_hex_number(address)&&addr<=0xFFFF){
        progaddr = addr;
        return 0;
    }
    return -1;
}

void read_element(char* dest, char* source, int num_elemnet){ //num_element갯수의 문자열을 source에서 dest로 읽어온다.
    strncpy(dest,source,num_elemnet);
    int len = strlen(source);
    for(int i = len;i<num_elemnet;i++){
        dest[i] = ' ';
    }
    dest[num_elemnet] = 0;
}


void push(ET* ESTAB,char* name, unsigned int addr, int is_control_section, int length){    // estab의 한줄에 push를 한다
    if(ESTAB->ET==NULL){
        ESTAB->ET = (ESTAB_TYPE*)calloc(sizeof(ESTAB_TYPE),1);
        ESTAB->size = 1;
    }
    else{
        ESTAB->size += 1;
        ESTAB->ET = (ESTAB_TYPE*)realloc(ESTAB->ET,sizeof(ESTAB_TYPE)*(ESTAB->size));
    }
    int idx = (ESTAB->size)-1;
    strcpy(ESTAB->ET[idx].name,name);
    ESTAB->ET[idx].addr = addr;
    ESTAB->ET[idx].is_control_section = is_control_section;
    ESTAB->ET[idx].length = length;
}


void delete_ET(ET ESTAB){ //다 쓴 estab을 지워준다.
    if(ESTAB.ET!=NULL) free(ESTAB.ET);
    ESTAB.ET = NULL;
    ESTAB.size = 0;
}

void print_ET(ET ESTAB){ //estab을 출력해준다.
    total_length = 0;
    printf("control       symbol    address length\n");
    printf("section       name\n");
    printf("---------------------------------------\n");
    for(int i = 0;i<ESTAB.size;i++){
        if(ESTAB.ET[i].is_control_section==control_section){
            printf("%s    ",ESTAB.ET[i].name);
            printf("%10s","");
        }
        else{
            printf("%10s","");
            printf("    %s",ESTAB.ET[i].name);
        }
        printf("    %04X",ESTAB.ET[i].addr);
        if(ESTAB.ET[i].is_control_section==control_section){
            printf("    %04X",ESTAB.ET[i].length);
            total_length += ESTAB.ET[i].length;
        }
        printf("\n");
    }
    printf("---------------------------------------\n");
    printf("               total length %04X\n",total_length);
}

int linking_loader_pass1(char filename[][COMMAND_LENGTH],int fileNum){ //lingking loader pass1을 구현한다.
    int startAddr = progaddr;
    ESTAB.ET = NULL;
    for(int i = 1;i<fileNum;i++){
        FILE* fp = fopen(filename[i],"r");
        if(fp==NULL){
            printf("FILE OPEN ERROR!\n");
            return -1;
        }
        char buf[300];
        char program_name[7];
        char addrs[7];
        int program_length = 0;
        int len = 0;
        while(fgets(buf,sizeof(buf),fp)){ //한 줄을 읽어온다.
            char ch = buf[0];
            len = strlen(buf);
            if(buf[len-1]=='\n') buf[--len] = 0;
            switch(ch){
                case 'H':// 헤더레코드일 때
                    read_element(program_name,buf+1,6);
                    read_element(addrs,buf+7,6);
                    startAddr += convert_hex_to_int(addrs);
                    read_element(addrs,buf+13,6);
                    //strncpy(addrs,buf+13,6); addrs[6] = 0;
                    program_length = convert_hex_to_int(addrs);
                    push(&ESTAB,program_name,startAddr,control_section,program_length); //estab에 push를 한다.
                    break;
                case  'D': //extern symbol을 define하는 record가 있을 때
                   for(int i = 0;i<len-1;i+=12){//for문을 돌며 이름 | address를 순차적으로 읽어온다.
                       read_element(program_name,buf+1+i,6);
                       read_element(addrs,buf+7+i,6);
                       push(&ESTAB,program_name,startAddr + convert_hex_to_int(addrs),not_control_section,-1);
                   }
                   break;
                case 'E'://end record일 때 프로그램의 시작 주소에 program 길이를 더한다.
                    startAddr += program_length;
            }
        }
        fclose(fp);
    }
    print_ET(ESTAB); 
    return 0;
}


int linking_loader_pass2(char filename[][COMMAND_LENGTH],int fileNum){
    for(int i = 1;i<fileNum;i++){
        FILE* fp = fopen(filename[i],"r");
        if(fp==NULL){
            printf("file Open error!");
            return -1;
        }
        char buf[300];
        char program_name[10];
        char addrs[10];
        char relation_len[5];
        int startAddr = 0;
        int currentAddr = 0;
        int byte_code = 0;
        int to_add = 0;
        ET temp;
        temp.ET = NULL;
        temp.size = 0;
        temp.size = 0;
        temp.ET = NULL;
        int reference_counter = 0;
        while(fgets(buf,sizeof(buf),fp)){
            char ch = buf[0];
            int len = strlen(buf);    
            if(buf[len-1]=='\n'){
                buf[--len] = 0;
            }
            
            switch(ch){
                case 'H': //헤더 레코드일 때
                    read_element(program_name,buf+1,6);
                    for(int i = 0;i<5;i++){
                        if(program_name[i]==0){
                            program_name[i] = ' ';
                        }
                    }
                    for(int i = 0;i<ESTAB.size;i++){
                        if(ESTAB.ET[i].is_control_section){
                            if(!strcmp(ESTAB.ET[i].name,program_name)){
                                currentAddr = ESTAB.ET[i].addr;
                                break;
                            }
                        }
                    }
                    push(&temp,program_name,currentAddr,1,-1); //나중에 사용하기 위해 템프 cstab을 만들어 넣어둔다.
                    break;
                case 'T': //텍스트 레코드일 때
                    read_element(addrs,buf+1,6); //시작주소
                    startAddr = currentAddr + convert_hex_to_int(addrs);
                    read_element(addrs,buf+7,2); //프로그램 길이
                    for(int i = 9;i<len;i+=2){
                        read_element(addrs,buf+i,2); //주소에 들어가야할 값을 입력받아서
                        memory[startAddr++] = convert_hex_to_int(addrs);//넣어준다.
                    }
                    break;
                case 'R':
                    for(int i = 1;i<len;i+=8){
                        read_element(relation_len,buf+i,2);
                        read_element(addrs,buf+2+i,6);
                        unsigned x = 0;
                        for(int j = 0;j<ESTAB.size;j++){
                            if(!strcmp(ESTAB.ET[j].name,addrs)){
                                x = ESTAB.ET[j].addr;
                                break;
                            }
                            
                        }
                        
                        push(&temp,addrs,x,convert_hex_to_int(relation_len),-1); 
                    }
                    break;
                case 'M':
                    read_element(addrs,buf+1,6);
                    read_element(program_name,buf+7,2);
                    byte_code = convert_hex_to_int(program_name);
                    read_element(relation_len,buf+9,3);
                    reference_counter = convert_hex_to_int(relation_len+1);
                    startAddr = currentAddr + convert_hex_to_int(addrs);
                    for(int i = 0;i<temp.size;i++){
                        if(temp.ET[i].is_control_section==reference_counter){
                            to_add = temp.ET[i].addr;
                            break;
                        }
                    }
                    int up = 0;
                    if(relation_len[0]=='-'){
                        to_add ^=0xFFFFFF;
                        to_add+=1;
                    }
                    if(byte_code%2==0){
                        for(int i = startAddr+byte_code/2-1;i>=startAddr;i-=1){   
                            int t = memory[i] + (to_add%256) + up;
                            if(t>=256) {up = t/256; t=(t+256)%256;}
                            else up = 0;
                            memory[i] = t;
                            to_add/=256;
                        }
                    }
                    else{
                        for(int i = startAddr+byte_code/2;i>=startAddr+1;i-=1){
                            int t = memory[i] + to_add%256 + up;
                            if(t>=256) {up = t/256; t=(t+256)%256;}
                            else up = 0;
                            memory[i] = t;
                            to_add/=256;
                        }
                        memory[startAddr] = memory[startAddr] + (to_add + up)%16;
                    }

                    break;
            }
        }
        delete_ET(temp);
        fclose(fp);
    }
    return 0;
}

int linking_loader(char filename[][COMMAND_LENGTH],int fileNum){
    int x = 0;
    x = linking_loader_pass1(filename,fileNum);
    if(x!=-1) x = linking_loader_pass2(filename,fileNum);
    delete_ET(ESTAB);
    PC = progaddr;
    L = total_length;
    return x;
}