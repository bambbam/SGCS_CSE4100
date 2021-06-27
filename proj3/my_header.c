//my_header.c

#include "my_header.h"

/**********************************
함수 : is_valid_hex_number
목적 : valid한 16진수인지를 확인한다.
return : 
valid한 16진수이면 => true
valid한 16진수가 아니면 => false
************************************/
bool is_valid_hex_number(char* test){
    int len = strlen(test);
    if(len==0) return false;
    for(int i = 0;i<len;i++){
        if(test[i]<='9'&&test[i]>='0') continue;
        if(test[i]<='f'&&test[i]>='a') continue;
        if(test[i]<='F'&&test[i]>='A') continue;
        return false;
    }
    return true;
}

/**********************************
함수 : is_valid_value
목적 : 0xFF이하의 valid한 16진수인지를 확인한다.
return : 
0xFF이하의 valid한 16진수이면 => true
0xFF이하의 valid한 16진수가 아니면 => false
************************************/
bool is_valid_value(char* test){
    int x = convert_hex_to_int(test);
    return x<=0xFF&&is_valid_hex_number(test);
}


/**********************************
함수 : is_valid_address
목적 : valid한 address인지를 확인한다.
return : 
valid한 address이면 => true
valid한 address가 아니면 => false
************************************/
bool is_valid_address(char* test){
    return is_valid_hex_number(test);
}

/**********************************
함수 : convert_hex_to_int
목적 : char*형태의 16진수를 int꼴의 10진수로 바꾼다.
return : 
char*형태의 16진수에 해당하는 int 값
************************************/
int convert_hex_to_int(char* todo){
    int len = strlen(todo);
    int ret= 0 ;
    for(int i = 0;i<len;i++){
        ret *= 16;
        if(todo[i]<='9'&&todo[i]>='0') ret+=todo[i]-'0';
        if(todo[i]<='z'&&todo[i]>='a') ret+=todo[i]-'a'+10;
        if(todo[i]<='Z'&&todo[i]>='A') ret+=todo[i]-'A'+10;
    }
    return ret;
}

/**********************************
함수 : help()
목적 : 우리의 shell 프로그램이 할 수 있는 명령어의 집합을 출력한다.
return : 
없음.
************************************/
void help(){
    const int instruction_size = 19;
    const char instruction_list[][COMMAND_LENGTH] = {
        "h[elp]", 
        "d[ir]",
        "q[uit]",
        "hi[story]",
        "du[mp] [start, end]",
        "e[dit] address, value",
        "f[ill] start, end, value",
        "reset",
        "opcode mnemonic",
        "opcodelist",
        "assemble filename",
        "type filename",
        "symbol",
        "progaddr address"
        "loader [object filename1] [object filename2] [...]",
        "bp address",
        "bp clear",
        "bp",
        "run"
    };
    for(int i = 0;i<instruction_size;i++){
        printf("%s\n",instruction_list[i]);
    }
    return;
}
/**********************************
함수 : dir()
목적 : 현재 디렉토리 내의 프로그램들을 상태와 함께 출력한다.
directory file이면 /를
executable file이면 *을 
둘 다 아니면 아무것도 출력하지 않는다.
return : 
없음.
************************************/
void dir(){
    DIR* curdir = opendir(".");
    if(!curdir){
        printf("Cannot Open Current Directory!\n");
        return;
    }
    struct dirent* iter;
    struct stat buf;
    int t = 0;
    while((iter = readdir(curdir))!=NULL){
        stat(iter->d_name,&buf);
        if(S_ISDIR(buf.st_mode)) printf("%19s/",iter->d_name); //디렉토리
        else if(buf.st_mode&S_IXUSR) printf("%19s*",iter->d_name); //실행파일
        else printf("%20s",iter->d_name); //둘 다 아님
        t++;
        if(t%4==0) printf("\n");
    }
    if(t%4!=0) printf("\n");
    closedir(curdir);
}


/**********************************
함수 : quit()
목적 : 프로그램을 종료한다. 동적할당 받았던 것들을 free해준다.
return : 
없음.
************************************/
void quit(){
    is_quit = true;
    free_hstry();
    free_hash_table();
    delete_symtab(temp_symtab);
    delete_symtab(symtab);
    delete_line(lines);
}

/**********************************
함수 : manufacture_instruction(char current_instruction[COMMAND_LENGTH])
목적 : current_instruction을 파싱하여 valid한 instruction이 들어왔는지 확인하고, 
      valid한 instruction이 들어왔다면, 각 명령어에 맞는 함수를 실행시킨다.
return : 
valid한 instruction이라면 true
invalid한 instruction이라면 false
************************************/
bool manufacture_instruction(char current_instruction[COMMAND_LENGTH]) {
    const int max_operand = 5;
    


    int idx = 0;
    int prev_idx = 0;
    int len = strlen(current_instruction);
    current_instruction[--len] = 0;
    char manufactured_instruction[5][COMMAND_LENGTH] = {0};
    int num_instruction = 0;
    //맨 앞에 있는 공백, 탭을 지우기 시작
    bool end_of_instruction = false;
    prev_idx = idx;
    for (; idx < len; idx++) {
        if (current_instruction[idx] != ' ' && current_instruction[idx] != '\t') {
            break;
        }
    }
    if (idx == len) return false; //공백, 탭만으로 instruction이 구성되어있다는 뜻.

    //맨 앞의 공백, 탭을 다 건너 뛰었으며, idx부터 첫번째 instruction을 받는다.
    prev_idx = idx;
    for (; idx < len; idx++) {
        if (current_instruction[idx] == ' ' || current_instruction[idx] == '\t' || current_instruction[idx] == ',') {
            break;
        }
    }
    if (idx == len) end_of_instruction = true;
    if (!end_of_instruction && current_instruction[idx] == ',') {//첫번째 instruction 바로 뒤에 ,가 왔다는 뜻. 첫번째 instruction은 구분자가 콤마가 되어서는 안된다.
        return false;
    }
    strncpy(manufactured_instruction[num_instruction++], current_instruction + prev_idx, (idx - prev_idx));
    //previdx~idx-1까지의 첫번쨰 instruction.



    if(!strcmp(manufactured_instruction[0],"loader")){
        int num = sscanf(current_instruction,"%s %s %s %s %s",manufactured_instruction[0],manufactured_instruction[1],manufactured_instruction[2],manufactured_instruction[3],manufactured_instruction[4]);
        if(num==5){
            return false;
        }
        for(int i = 0;i<5;i++){
            int len = strlen(manufactured_instruction[i]);
            for(int j = 0;j<len;j++){
                if(manufactured_instruction[i][j]==',') return false;    
            }
        }
        num_instruction = num;
        int t = linking_loader(manufactured_instruction,num_instruction);
        if(t==0) {
            push_history_node(hstry,manufactured_instruction,num_instruction);
            A = X = B = S = T = SW = 0;
        }
        return true;
    }




    
    //구분자로 탭 또는 띄어쓰기가 오는 것을 지운다.
    prev_idx = idx;
    for (; idx < len; idx++) {
        if (current_instruction[idx] != ' ' && current_instruction[idx] != '\t') {
            break;
        }
    }
    if (!end_of_instruction && current_instruction[idx] == ',') return false;//구분자로 ,가 와서는 안된다.
    if (idx == len) end_of_instruction = true;


    while (!end_of_instruction&&num_instruction<max_operand) {
        //띄어쓰기, 탭, comma의 구분자가 나오기 전까지 idx를 증가시킨다.
        prev_idx = idx;
        for (; idx < len; idx++) {
            if (current_instruction[idx] == ' ' || current_instruction[idx] == '\t' || current_instruction[idx] == ',') {
                break;
            }
        }
        strncpy(manufactured_instruction[num_instruction++], current_instruction + prev_idx, (idx - prev_idx));
        //previdx~idx-1까지가 이번에 파싱한 instruction이다.
        if (idx == len) break;
        int num_comma = 0;
        prev_idx = idx;
        //구분자들을 모두 건너뛴다.
        for (; idx < len; idx++) {
            if (current_instruction[idx] != ' ' && current_instruction[idx] != '\t' && current_instruction[idx] != ',') {
                break;
            }
            else if (current_instruction[idx] == ',') num_comma++;
        }
        if(idx==len&&num_comma==0) break;//구분자안에 ,가 없고, 문장의 끝에 도달했다면, instruction이 끝난 것이다.
        if (num_comma != 1 || (num_comma == 1 && idx == len)) return false;//문장의 끝에 도달했으나 콤마가 있거나, 콤마의 갯수가 1개가 아니라면 잘못된 입력이다.
    }
    if(num_instruction==max_operand) return false;//너무나 많은 입력이 들어왔다면 이또한 잘못된 입력이다.

/******************************************문자열 파싱 끝// 문자열 처리시작**********************************************************************************************************/
    bool success = false;
    if(!strcmp("help",manufactured_instruction[0])||!strcmp("h",manufactured_instruction[0])){
        if(num_instruction==1) {
            help();
            success = true;
            push_history_node(hstry,manufactured_instruction,num_instruction);
        }
    }
    else if(!strcmp("dir",manufactured_instruction[0])||!strcmp("d",manufactured_instruction[0])){
        if(num_instruction==1) {
            success = true;
            dir();
            push_history_node(hstry,manufactured_instruction,num_instruction);
        }
    }
    else if(!strcmp("quit",manufactured_instruction[0])||!strcmp("q",manufactured_instruction[0])){
        if(num_instruction==1) {
            success = true;
            quit();
        }
    }
    else if(!strcmp("hi",manufactured_instruction[0])||!strcmp("history",manufactured_instruction[0])){
        if(num_instruction==1){
            success = true;
            push_history_node(hstry,manufactured_instruction,num_instruction);
            print_history(hstry);
        }
    }
    else if(!strcmp("du",manufactured_instruction[0])||!strcmp("dump",manufactured_instruction[0])){
        if(num_instruction==1){
            success = dump(-1,-1,0);
            if(success) push_history_node(hstry,manufactured_instruction,num_instruction);
        }
        else if(num_instruction==2&&is_valid_address(manufactured_instruction[1])){
            success = dump(convert_hex_to_int(manufactured_instruction[1]),-1,1);
            if(success) push_history_node(hstry,manufactured_instruction,num_instruction);
        }
        else if(num_instruction==3&&is_valid_address(manufactured_instruction[1])&&is_valid_address(manufactured_instruction[2])){
            success = dump(convert_hex_to_int(manufactured_instruction[1]),convert_hex_to_int(manufactured_instruction[2]),2);
            if(success) push_history_node(hstry,manufactured_instruction,num_instruction);
        }
    }
    else if(!strcmp("e",manufactured_instruction[0])||!strcmp("edit",manufactured_instruction[0])){
        if(num_instruction==3&&is_valid_address(manufactured_instruction[1])&&is_valid_value(manufactured_instruction[2])){
            success = edit(convert_hex_to_int(manufactured_instruction[1]),convert_hex_to_int(manufactured_instruction[2]));
            if(success) push_history_node(hstry,manufactured_instruction,num_instruction);
        }
    }
    else if(!strcmp("f",manufactured_instruction[0])||!strcmp("fill",manufactured_instruction[0])){
        if(num_instruction==4&&is_valid_address(manufactured_instruction[1])&&is_valid_address(manufactured_instruction[2])&&is_valid_value(manufactured_instruction[3])){
            success = fill(convert_hex_to_int(manufactured_instruction[1]),
                                convert_hex_to_int(manufactured_instruction[2]),
                                convert_hex_to_int(manufactured_instruction[3]));
            if(success) push_history_node(hstry,manufactured_instruction,num_instruction);
        }
    }
    else if(!strcmp("r",manufactured_instruction[0])||!strcmp("reset",manufactured_instruction[0])){
        if(num_instruction==1){
            success = reset();
            if(success) push_history_node(hstry,manufactured_instruction,num_instruction);
        }
    }
    else if(!strcmp("opcode",manufactured_instruction[0])){
        if(num_instruction==2){
           success = mnemonic(manufactured_instruction[1]);
           if(success) push_history_node(hstry,manufactured_instruction,num_instruction);
        }
    }
    else if(!strcmp("opcodelist",manufactured_instruction[0])){
        if(num_instruction==1){
            success = true;
            opcodelist();
            if(success) push_history_node(hstry,manufactured_instruction,num_instruction);
        }
    }
    else if(!strcmp("assemble",manufactured_instruction[0])){
        if(num_instruction==2){
            success = true;
            int t = assemble(manufactured_instruction[1]); //자체에서 성공 판별함.
            if(asm_file){
                fclose(asm_file); asm_file=NULL;
            }
            if(t==0) push_history_node(hstry,manufactured_instruction,num_instruction);
        }
    }
    else if(!strcmp("symbol",manufactured_instruction[0])){
        if(num_instruction==1){      
            print_symtab(symtab);
            success = true;
            push_history_node(hstry,manufactured_instruction,num_instruction);
        }
    }
    else if(!strcmp("type",manufactured_instruction[0])){
        if(num_instruction==2){
            int t = type(manufactured_instruction[1]);
            success = true;
            if(t==0) push_history_node(hstry,manufactured_instruction,num_instruction);
        }
    }
    /*
    else if(!strcmp("loader",manufactured_instruction[0])){
        if(num_instruction<=4&&num_instruction>=1){
            int t = linking_loader(manufactured_instruction,num_instruction);
            success = true;
            if(t==0) {
                push_history_node(hstry,manufactured_instruction,num_instruction);
                A = X = B = S = T = SW = 0;
            }
        }
    }*/
    else if(!strcmp("progaddr",manufactured_instruction[0])){
        if(num_instruction==2){
            int t = set_progaddr(manufactured_instruction[1]);
            success = true;
            if(t==0) push_history_node(hstry,manufactured_instruction,num_instruction);
        }
    }
    else if(!strcmp("run",manufactured_instruction[0])){
        if(num_instruction==1){
            int t = run();
            success = true;
            if(t==0) push_history_node(hstry,manufactured_instruction,num_instruction);
        }
    }
    else if(!strcmp("bp",manufactured_instruction[0])){
        if(num_instruction==1){
            print_bp();
            success = true;
            push_history_node(hstry,manufactured_instruction,num_instruction);
        }
        if(num_instruction==2){
            int t = bp(manufactured_instruction[1]);
            success = true;
            if(t==0) push_history_node(hstry,manufactured_instruction,num_instruction);
        }
    }
    if(!success) return false;


    return true;
}

/**********************************
함수 : push_history_node(history* st, char value[5][COMMAND_LENGTH], int num_element){
목적 : valid한 instruction이 들어왔다면 그것을 historynode에 넣어줘야한다.
잘 정돈된 형태로 링크드 리스트에 잘 넣어준다.
return : 
없음.
************************************/
void push_history_node(history* st, char value[5][COMMAND_LENGTH], int num_element){
    history_node* node = (history_node*)calloc(1,sizeof(history_node));
    
    node->num_element = num_element;
    for(int i = 0;i<num_element;i++){
        node->values[i] = (char*)calloc(strlen(value[i])+1,sizeof(char));
        strcpy(node->values[i],value[i]);
    }
    
    if(st->head==NULL){
        node->idx = 1;
        st->head = node;
        st->tail = node;
    }

    else{
        node->idx = (st->tail->idx)+1;
        st->tail->next = node;
        st->tail = node;
    }

}

/**********************************
함수 : print_history(history* st)
목적 : 지금까지 넣어왔던 히스토리를 출력해준다. 링크드 리스트를 순회하며 잘 주면 된다.
return : 
없음.
************************************/
void print_history(history* st){
    if(st->head==NULL) return;
    history_node* head = st->head;
    while(head!=NULL){
        printf("%d\t", head->idx);
        printf("%s",head->values[0]);
        for(int i = 1;i<(head->num_element)-1;i++){
            printf(" %s,",head->values[i]);
        }
        if(head->num_element!=1) printf(" %s\n",head->values[(head->num_element)-1]);
        else printf("\n");
        head = head->next;
    }
}
/**********************************
함수 : print_history(history* st)
목적 : 프로그램이 끝났다면, 동적할당했던 메모리를 free를 해준다.
return : 
없음.
************************************/
void free_hstry(){
    if(hstry==NULL) return;
    history_node* head = hstry->head;
    while(head!=NULL){
        for(int i = 0;i<(head->num_element);i++){
            free(head->values[i]);
        }
        history_node* to_delete = head;
        head = head->next;
        free(to_delete);
    }
    free(hstry);
    hstry = NULL;
}



int type(const char* filename){
    FILE* fp = fopen(filename,"r");
    if(fp==NULL){
        printf("Cannot Open FILE\n");
        return -1;
    }
    char buf[200];
    while(fgets(buf,sizeof(buf),fp)){
        int len = strlen(buf);
        fputs(buf,stdout);
        if(buf[len-1]!='\n') printf("\n");
    }
    return 0;
}

