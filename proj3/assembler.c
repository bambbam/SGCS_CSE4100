#include "assembler.h"
#include "opcode.h"
lineTab** lines=NULL;
int lineNum=0;
char* lst_filename = NULL;
char* obj_filename = NULL;
FILE* asm_file=NULL, *lst_file=NULL, *obj_file=NULL;
/**********************************
함수 : hash_sym(const char* todo)
목적 : 
심볼테이블 인덱싱을 위한 해싱을 수행한다.
97개의 해싱중 하나가 선택된다.
return : 다 더해서 97로 나눈 나머지가 return된다.
************************************/
int hash_sym(const char* todo){
    int len = strlen(todo);
    int ret = 0;
    for(int i = 0;i<len;i++){
        ret += todo[i];
        ret %= 97;
    }
    return ret;
}
/**********************************
함수 :  push_symtab(const char* todo, int lc ,symbol_element* symtab[97])
목적 : 
심볼테이블에 새로운 심볼을 집어넣기 위한 연산이 수행된다.
각각의 링크드 리스트는 정렬되어 있으며, 심볼테이블에 중복되는 원소가 나온다면, 에러를 return한다.
return : 에러면 -1, 아니면 0을 return
************************************/
int push_symtab(const char* todo, int lc ,symbol_element* symtab[97]){

    if(get_code(todo)!=-1){
        return -2;
    }//opcode와 symbol이 겹치는 경우

    int x = hash_sym(todo);//해시값
    symbol_element* toinsert = (symbol_element*)calloc(1,sizeof(symbol_element)); //이번에 넣을 값
    toinsert->location_counter = lc;
    toinsert->symbol = (char*)calloc(strlen(todo)+1,sizeof(char));
    strcpy(toinsert->symbol,todo);  //저장

    if(symtab[x]==NULL){
        symtab[x] = toinsert;//NULL이라면 헤드가 된다.
    }
    else{    
        symbol_element* root = symtab[x];
        if(strcmp(root->symbol,todo)>0){ //만약 이번에 넣을 값 < 첫 원소값이라면
            toinsert->next = symtab[x];
            symtab[x] = toinsert;   
        }
        else if(strcmp(root->symbol,todo)==0){ //만약 이번에 넣을 값 == 첫원소값이라면
            free(toinsert->symbol);
            free(toinsert);
            return -1;
        }
        else{
            while(root->next!=NULL){
                if(strcmp(root->symbol,todo)>0){ // root < 이번에 넣을 값 < root->next라면
                    break;
                }
                if(strcmp(root->symbol,todo)==0){ //이번에 넣을 값 == root->next라면
                    return -1;
                }
            }
            toinsert->next = root->next; //root와 root->next 사이에 집어넣는다.
            root->next = toinsert;
        }
    }
    return 0;
}
/**********************************
함수 :  delete_symtab(symbol_element* symtab[97])
목적 : 
심볼테이블이 더 이상 쓸모없어졌을 때 심볼테이블을 동적할당해제해주기 위함.
return : void
************************************/
void delete_symtab(symbol_element* symtab[97]){
    for(int i = 0;i<97;i++){
        symbol_element* root = symtab[i]; //순회하기 위함
        while(root!=NULL){
            symbol_element* temp = root; //지울 친구를 가지고
            root = root->next; // 포인터를 옆으로 옮겨둔 후
            if(temp->symbol) {free(temp->symbol);}
            free(temp); //할당 해제
        }
        symtab[i] = NULL; // 할당해제가 끝난 친구는 NULL로 바꿔준다.
    }
}


/**********************************
함수 :  delete_symtab(symbol_element* symtab[97])
목적 : 
심볼테이블을 출력해주기 위함.
symbol명령을 수행해주기 위함이다. 정렬을 해준다.
return : symbol table이 생성된 적이 없다면 -1, 잘 출력했다면 0
************************************/
int print_symtab(symbol_element* symtab[97]) {
    symbol_element** arr = NULL; //심볼테이블 링크드 리스트를 저장하기 위한 배열
    int num_symbol = 0;
    for (int i = 0; i < 97; i++) {
        symbol_element* root = symtab[i]; //링크드 리스트 순회
        while (root != NULL) {
            num_symbol++;
            if (arr == NULL) arr = (symbol_element**)malloc(sizeof(symbol_element*) * num_symbol); 
            else arr = (symbol_element**)realloc(arr,sizeof(symbol_element*)*num_symbol);
            arr[num_symbol - 1] = root; //배열에 하나씩 넣는다.
            root = root->next;
        }
    }
    if(num_symbol==0){//에러
        printf("No exsting Symtab!!\n");
        return -1;
    }
    for (int i = 0; i < num_symbol; i++) {
        int swapidx = i;
        for (int j = i + 1; j < num_symbol; j++) {
            if (strcmp(arr[swapidx]->symbol, arr[j]->symbol) > 0) {
                swapidx = j;
            }
        }//swapidx에는 i~num_symbol-1까지의 가장 작은 값이 저장된다.
        symbol_element* c = arr[i];
        arr[i] = arr[swapidx];
        arr[swapidx] = c;
    }
    printf("symbol\n");
    fflush(stdout);
    for (int i = 0; i < num_symbol; i++) {
        printf("\t%s\t%04X\n", arr[i]->symbol, arr[i]->location_counter);
        fflush(stdout);
    }//출력 잘 해주고
    free(arr); //동적할당 해제해준다.
    return 0;
}

/**********************************
함수 :  void delete_line()
목적 : 
라인의 정보가 더이상 쓸모가 없어지는 경우 라인들을 모두 동적할당을 해제해준다.
return : void
************************************/
void delete_line(){
    if(lines==NULL) return;
    for(int i = 0;i<lineNum;i++){
        if(lines[i]->objectCode) {free(lines[i]->objectCode);}
        if(lines[i]->line) {free(lines[i]->line);}
        free(lines[i]);
    }
    free(lines);
    lineNum = 0;
    lines = NULL;
}


/**********************************
함수 : void push_line(char* buf, int location_counter, int is_symbolic, int format, int assembelr_directives);
목적 : 
c++  vector의 push_back과 비슷한 역할을 하기를 바랬다.
lines배열의 맨 끝에 원하는 값을 하나 넣는다.
return : void
************************************/
void push_line(char* buf, int location_counter, int is_symbolic, int format, int assembelr_directives){
    lineNum++;
    if(lines==NULL){
        lines = (lineTab**)calloc(sizeof(lineTab*),1); //아직 할당된게 없다면
    }
    else{
        lines = (lineTab**)realloc(lines,sizeof(lineTab*)*lineNum); //하나 늘린다.
    }
    lines[lineNum-1] = (lineTab*)calloc(sizeof(lineTab),1);
    lines[lineNum-1]->isSymbol = is_symbolic;
    lines[lineNum-1]->assembler_directives = assembelr_directives;
    lines[lineNum-1]->location_counter = location_counter;
    lines[lineNum-1]->line = (char*)calloc(sizeof(char),strlen(buf)+1);
    strcpy(lines[lineNum-1]->line,buf);
    lines[lineNum-1]->format = format;
    lines[lineNum-1]->need_modification_record = 0;//값을 넣는다.
}

/**********************************
함수 : int assemble(const char* filename);
목적 : 
assemble의 pass1을 수행하고, 그 결과를 lines라는 배열에 차곡차곡 쌓는다.
다 한 후 pass2를 수행한다.
return : 에러가 있다면 -1을 return하고, 에러가 없다면 pass2의 return값을 return한다.
************************************/
int assemble(const char* filename){
    int len_filename = strlen(filename);
    if(strcmp(filename+len_filename-4,".asm")) {
        printf("file name is not valid\n");
        return -1;
    }
    asm_file = fopen(filename,"r"); //파일을 하나 연다.
    if(asm_file==NULL){
        printf("file open error!\n");
        return -1;
    }
    delete_symtab(temp_symtab); //혹시나 아직 temp symbolic table이 안비워져있을까봐.
    delete_line(); //혹시나 lines배열이 아직 안비워져있을까봐.
    char buf[100]; //입력을 받는 buffer
    int linenum = 0; //몇번째 라인인가
    int location_counter = 0; //location counter는 쌓이는 것이기 때문에 변수로 관리를 한다.

    while(fgets(buf,sizeof(buf),asm_file)!=NULL){
        int buf_len = strlen(buf);
        int ch = 0;
        for(int i = 0;i<buf_len;i++){
            if(buf[i]!=' '&&buf[i]!='\n'&&buf[i]!='\t'){ch = 1; break;}
        }
        if(!ch) continue;
        if(buf[buf_len-1]=='\n') {buf[--buf_len] = 0;} //개행 문자를 지워준다.
        linenum += 5;
        int is_comment = 0; //이번에 받은친구가 comment인가?
        if(buf[0]=='.') is_comment = 1;
        int is_symbolic = 1; //이번에 받은 친구가 symbolic이 포함되는가?
        if(buf[0]==' '||buf[0]=='\t') is_symbolic = 0;
        if(is_comment){
            push_line(buf,location_counter,is_symbolic,0,0); //comment라면 딱히 처리해줄게 없다. 
            continue;
        }
        char statements[5][35]  = {0}; //buffer를 쪼개준다. 개행을 기준으로.
        int num_statements = -1;
        int idx = 0;
        int MAX_NUM_STATEMENTS = 5;
        for(int i = 0;i<buf_len&&num_statements<MAX_NUM_STATEMENTS;i++){
            if(buf[i]!=' '&&buf[i]!='\t'){
               if(idx==0) num_statements++;
               statements[num_statements][idx++] = buf[i];
            }
            else{
                idx = 0;
            }
        }
        num_statements++; //0~num_statements-1에 statement들이 들어간다.

        if(num_statements==MAX_NUM_STATEMENTS){
            error_process(ELEMENT_NUMBER_ERROR,linenum, buf);
            return -1;
        }


        int start = 0;
        int whaterror = 0;
        if(is_symbolic) { //심볼이 있다면
            start = 1;
            if((whaterror = push_symtab(statements[0],location_counter, temp_symtab))!=0){//심볼릭 테이블에 넣었는데 중복되는게 있으면
                if(whaterror==-1) error_process(SAME_SYMBOL_ERROR,linenum,buf); //같은 심볼이 있다고 에러를 출력하고, 동적할당을 좀 처리해준다음에 -1을 리턴하자.
                if(whaterror==-2) error_process(SAME_OPCODE_SYMBOL,linenum,buf);
                return -1;
            }
        }
        if(!strcmp(statements[start],"START")){
            char* iter = statements[start+1];
            location_counter = 0;
            int flag = 0;
            while(*iter!='\0'){
                location_counter *= 16;
                if(*iter>='0'&&*iter<='9') location_counter+=(*iter)-'0';
                else if(*iter>='A'&&*iter<='F') location_counter+=(*iter)-'A'+10;
                else {flag = 1; break;}
                iter++;
            }
            if(flag){error_process(CANNOT_HEX_IN_START,linenum,buf);}
            push_line(buf,location_counter,is_symbolic,asm_dir,1); 
            continue;
        }


        if(!strcmp(statements[start],"END")){
            push_line(buf,location_counter,is_symbolic,asm_dir,2);
            break;
        }

        else if(!strcmp(statements[start],"WORD")){
            push_line(buf,location_counter,is_symbolic,asm_dir,4);
            location_counter += 3;//word size만큼 +
        }
        else if(!strcmp(statements[start],"RESW")){
            push_line(buf,location_counter,is_symbolic,asm_dir,6);
            if(is_valid_Number(statements[start+1],1<<16)){
                location_counter += 3*atoi(statements[start+1]);//3*옆에 있는 숫자만큼 +
            }
            else{
                error_process(INVALID_OPERAND,linenum,buf);
                return -1;
            }
        }
        else if(!strcmp(statements[start],"RESB")){
            push_line(buf,location_counter,is_symbolic,asm_dir,5);
            if(is_valid_Number(statements[start+1],1<<20)){
                location_counter += 1*atoi(statements[start+1]);//1*옆에 있는 숫자만큼 +
            }
            else{
                error_process(INVALID_OPERAND,linenum,buf);
                return -1;
            }
        }
        else if(!strcmp(statements[start],"BYTE")){
            push_line(buf,location_counter,is_symbolic,asm_dir,3);
            if(statements[start+1][0]=='C'){
                int len = strlen(statements[start+1]);
                location_counter+=len-3;//1*문자열의 길이만큼 +
            }
            else if(statements[start+1][0]=='X'){
                int len = strlen(statements[start+1]);
                location_counter+=(len-3)/2+(len-3)%2;//1*필요한 바이트만큼 +
            }
            else{
                error_process(INVALID_OPERAND,linenum,buf);
                return -1;
            }
        }
        else if(!strcmp(statements[start],"BASE")){
            push_line(buf,location_counter,is_symbolic,asm_dir,7);
            location_counter = location_counter;//base는 location counter에 영향을 안받는다.
        }
        else if(!strcmp(statements[start],"NOBASE")){
            push_line(buf,location_counter,is_symbolic,asm_dir,NOBASE);
            location_counter = location_counter;
        }
        
        else{
            char* x = get_format(statements[start]);
            if(statements[start][0]=='+') {
                push_line(buf,location_counter,is_symbolic,4,0);
                location_counter += 4;//format4
            } 
            else if(!strcmp(x,"2")){
                push_line(buf,location_counter,is_symbolic,2,0);
                location_counter += 2;//format2
            }
            else if(!strcmp(x,"1")){
                push_line(buf,location_counter,is_symbolic,1,0);
                location_counter += 1;//format1
            }
            else if(!strcmp(x,"3/4")){
                push_line(buf,location_counter,is_symbolic,3,0);
                location_counter += 3;    //format3
            }
            else{
                error_process(INVALID_OP_CODE,linenum,buf); //아무데도 해당 안하면 당연히 에러
                return -1;
            }
        }
    }
    //모든 시련을 견뎌냈다면, pass2로 간다.
    return path_two(filename);
}

/**********************************
함수 : symbol_element* find_sym(const char* x, symbol_element* symtab[97]);
목적 : 심볼 테이블에서 x에 해당하는 심볼에 관련된 정보를 받아온다.
return : 정보가 있으면 그 symbol_element를, 없으면 NULL를 return
************************************/
symbol_element* find_sym(const char* x, symbol_element* symtab[97]){
    int t = hash_sym(x);//해시값
    symbol_element* head = symtab[t];//순회하기 위한 변수
    while(head!=NULL){
        if(!strcmp(head->symbol,x)){//있다면
            return head;//그놈을 return
        }
        head = head->next;
    }
    return NULL;
}

/**********************************
함수 : int regToNumber(const char* a){
목적 : 레지스터에 해당하는 숫자를 return받기 위해 사용한다.
return : 레지스터가 있으면 그 레지스터에 해당하는 값을, 없으면 -1을 return
************************************/
int regToNumber(const char* a){
    if(!strcmp(a,"A")) return 0;
    if(!strcmp(a,"X")) return 1;
    if(!strcmp(a,"L")) return 2;
    if(!strcmp(a,"PC")) return 8;
    if(!strcmp(a,"SW")) return 9;
    if(!strcmp(a,"B"))  return 3;
    if(!strcmp(a,"S")) return 4;
    if(!strcmp(a,"T")) return 5;
    if(!strcmp(a,"F")) return 6;
    return -1;
}



/**********************************
함수 : char* make_object_code(unsigned int x, int operand_length){
목적 : unsigned int 꼴의 objectcode를 char*꼴로 바꿔준다.
return : 헤당하는 objectcode를 return
************************************/
char* make_object_code(unsigned int x, int operand_length){
    char* objectCode = (char*)calloc(sizeof(char),operand_length+1);
    unsigned int temp = x;
    for(int i = operand_length-1;i>=0;i--){
        int k = temp%16;
        if(k>=0&&k<=9) objectCode[i] = k+'0';
        else objectCode[i] = k+'A'-10;
        temp/=16;
    }
    return objectCode;
}

/**********************************
함수 : void error_process(int error_type, int lineNUmber){
목적 : 에러를 처리하기 위해 사용한다.
return : void
************************************/

void error_process(int error_type, int lineNUmber, const char* line){
    printf("%dth line : %s\n",lineNUmber/5, line);
    switch (error_type)
    {
    case ELEMENT_NUMBER_ERROR:
        printf("element number is more or less than required\n");
        break;
    case CANNOT_FIND_SYMBOLIC:
        printf("cannot find symbolic element. symbolic table error\n");
        break;
    case CANNOT_FIND_REGISTER:
        printf("cannot find this register.\n");
        break;
    case CANNOT_RELATIVE_ADDR:
        printf("cannot save addr as relative addr\n");
        break;
    case SAME_SYMBOL_ERROR:
        printf("Same Symbol in SYMTAB\n");
        break;
    case INVALID_OP_CODE:
        printf("invalid opcode founded\n");
        break;
    case SAME_OPCODE_SYMBOL:
        printf("symbol and opcode cannot be same\n");
        break;
    case NO_X_BIT:
        printf("only X bit can be here 3/4 format\n");
        break;
    case INVALID_OPERAND:
        printf("INVALID operand founded\n");
    default:
        break;
    }
    fflush(stdout);
    if(lst_filename!=NULL) {remove(lst_filename); free(lst_filename); lst_filename = NULL;}
    if(obj_filename!=NULL) {remove(obj_filename); free(obj_filename); obj_filename = NULL;}
    if(lst_file){fclose(lst_file); lst_file = NULL;}
    if(obj_file){fclose(obj_file); obj_file = NULL;}
    delete_symtab(temp_symtab);
    delete_line();
}

int is_valid_Number(char* t, int upper){
    char* num = t;
    int x = 0;
    int numlen = strlen(num);
    if(numlen>7) return 0; //숫자가 너무 크다
    while(*num != '\0'){
        if(!(*num<='9'&&*num>='0')) return 0; //숫자가 아니다
        x*=10; x+=*num-'0';
        num++;
    }
    if(x<upper) return 1;
    return 0;
}

/**********************************
함수 : int path_two(const char* fileanme)
목적 : pass1에서 수행한 결과를 통해 obj, lst file을 만들기 위함이다.
return : 에러가 있다면 -1, 에러가 없다면 make_obj의 return값을 따른다.
************************************/
int path_two(const char* fileanme){
    int len = strlen(fileanme);
    lst_filename = (char*)calloc(sizeof(char),len+1);
    obj_filename = (char*)calloc(sizeof(char),len+1);
    strcpy(lst_filename,fileanme);
    strcpy(obj_filename,fileanme);
    lst_filename[len-1] = 't';
    lst_filename[len-2] = 's';
    lst_filename[len-3] = 'l';
    obj_filename[len-1] = 'j';
    obj_filename[len-2] = 'b';
    obj_filename[len-3] = 'o';
    lst_file = fopen(lst_filename,"w"); //lstfile을 하나 만들어준다.
    char tmp[40], symbolic[40], opcode[40], operand1[40], operand2[40]; //line의 값을 나눠주기 위함.
    char dump[1] = {0}; //출력을 양식에 맞춰서 해주기 위함.
    int baseValue = -1; //base relative에서 참조하는 값.
    char* objectCode; //object Code가 만들어지면 저장되는 변수.
    for(int i = 0;i<lineNum;i++){
        lineTab* thisLine = lines[i]; //이번 라인을 읽었는데
        if(lines[i]->format == comments){ // comment라면 lst file에 고민없이 써준다.
            fprintf(lst_file,"%-6d %4s     %-30s\n",(i+1)*5,dump,lines[i]->line);
            continue;
        }
        
        else if(lines[i]->format==asm_dir){ //assembler directive라면
            int num = 0;
            switch(lines[i]->assembler_directives){
                case START:
                case RESB:
                case RESW://start, resb, resw는 별게 없다. 같은 형식으로 출력해준다.
                    if(thisLine->isSymbol){
                        num = sscanf(thisLine->line,"%s %s %s %s",symbolic,opcode, operand1, tmp);
                        if(num!=3) {error_process(ELEMENT_NUMBER_ERROR,(i+1)*5,thisLine->line); return -1;} //element갯수가 안맞다면 error
                        if(find_sym(symbolic,temp_symtab)==NULL){
                            error_process(CANNOT_FIND_SYMBOLIC, (i+1)*5,thisLine->line);//symbolic이 없는 symbol이라면
                            return -1;
                        }
                    }
                    else{
                        num = sscanf(thisLine->line,"%s %s %s",opcode, operand1, tmp);
                        if(num!=2) {error_process(ELEMENT_NUMBER_ERROR,(i+1)*5,thisLine->line); return -1;}//element갯수가 안맞다면 error
                    }
                    fprintf(lst_file,"%-6d %04X     %-30s\n",(i+1)*5,lines[i]->location_counter,lines[i]->line);
                    break;
                case END:
                    num = sscanf(thisLine->line,"%s %s %s",opcode, operand1, tmp);
                    if(num!=2&&num!=1) {error_process(ELEMENT_NUMBER_ERROR,(i+1)*5,thisLine->line); return -1;}//element갯수가 안맞다면 error
                    fprintf(lst_file,"%-6d %4s     %-30s\n",(i+1)*5,dump,lines[i]->line);
                    break;
                case BYTE:
                    if(thisLine->isSymbol){
                        num = sscanf(thisLine->line,"%s %s %s %s",symbolic, opcode, operand1, tmp);
                        if(num != 3){      
                            error_process(ELEMENT_NUMBER_ERROR, (i+1)*5,thisLine->line);//element갯수가 안맞다면 error
                            return -1;
                        }
                        if(find_sym(symbolic,temp_symtab)==NULL){
                            error_process(CANNOT_FIND_SYMBOLIC, (i+1)*5,thisLine->line);//symbolic이 없는 symbol이라면
                            return -1;
                        }
                    }
                    else{
                        num = sscanf(thisLine->line,"%s %s %s",opcode, operand1, tmp);
                        if(num!=2){
                            error_process(ELEMENT_NUMBER_ERROR, (i+1)*5,thisLine->line);//element갯수가 안맞다면 error
                            return -1;
                        }
                    }
                    if(operand1[0]=='X'){//16진수에 대해 처리
                        int op_len = strlen(operand1);
                        objectCode = (char*)calloc(sizeof(char),op_len-2);
                        for(int i = 2;i<op_len-1;i++){
                            objectCode[i-2] = operand1[i];
                        }
                    }
                    else if(operand1[0]=='C'){//string에 대해 처리
                        int op_len = strlen(operand1);
                        objectCode = (char*)calloc(sizeof(char),2*(op_len-3)+1);
                        for(int i = 2;i<op_len-1;i++){
                            char c2 = operand1[i]%16;
                            char c1 = operand1[i]/16;
                            if(c1<10) c1 += '0';
                            else c1 += 'A'-10;
                            if(c2<10) c2 += '0';
                            else c2 += 'A'-10;
                            objectCode[(i-2)*2] = c1;
                            objectCode[(i-2)*2+1] = c2;
                        }
                    }
                    thisLine->objectCode = objectCode;//각각에 맞는 object code를 생성했다면
                    fprintf(lst_file,"%-6d %04X     %-30s %-30s\n",(i+1)*5, thisLine->location_counter, thisLine->line, objectCode);//출력
                    break;
                case WORD:
                    if(thisLine->isSymbol){
                        num = sscanf(thisLine->line, "%s %s %s %s",symbolic, opcode, operand1, tmp);
                        if(num != 3){
                            error_process(ELEMENT_NUMBER_ERROR, (i+1)*5,thisLine->line);//element갯수가 안맞다면 error
                            return -1;
                        }
                        if(find_sym(symbolic,temp_symtab)==NULL){
                            error_process(CANNOT_FIND_SYMBOLIC, (i+1)*5,thisLine->line);//symbolic이 없는 symbol이라면
                            return -1;
                        }
                    }
                    else{
                        int num = sscanf(thisLine->line,"%s %s %s",opcode, operand1, tmp);
                        if(num!=2){
                            error_process(ELEMENT_NUMBER_ERROR, (i+1)*5,thisLine->line);//element갯수가 안맞다면 error
                            return -1;
                        }
                    }
                    objectCode = (char*)calloc(sizeof(char),7); //무조건 3바이트를 넣어줘야한다.
                    int x = atoi(operand1);//operand1을 숫자로 바꿔준 후에
                    for(int i = 5;i>=0;i--){
                        char c1 = x%16;
                        x/=16;
                        if(c1<10) c1 += '0';
                        else c1 += 'A'-10;
                        objectCode[i] = c1;
                    }
                    thisLine->objectCode = objectCode;
                    fprintf(lst_file,"%-6d %04X     %-30s %-30s\n",(i+1)*5, thisLine->location_counter, thisLine->line, objectCode);
                    break;
                case BASE:
                    num = sscanf(thisLine->line,"%s %s %s",opcode,operand1,tmp);
                    if(num!=2){
                        error_process(ELEMENT_NUMBER_ERROR,(i+1)*5,thisLine->line);
                        return -1;
                    }
                    if(find_sym(operand1,temp_symtab)==NULL){
                        error_process(CANNOT_FIND_SYMBOLIC,(i+1)*5,thisLine->line);
                        return -1;
                    }
                    baseValue = find_sym(operand1,temp_symtab)->location_counter; //baseValue를 set해준다.
                    fprintf(lst_file,"%6s %4s     %-30s\n",dump, dump, thisLine->line);
                    break;
                case NOBASE:
                    num = sscanf(thisLine->line,"%s %s",opcode,tmp);
                    if(num!=1){
                        error_process(ELEMENT_NUMBER_ERROR,(i+1)*5,thisLine->line);
                        return -1;
                    }
                    baseValue = -1; //baseValue를 set해준다.
                    fprintf(lst_file,"%6s %4s     %-30s\n",dump, dump, thisLine->line);
                    break;
            }
        }
        else{
            int num,x,op1,op2;
            if(thisLine->isSymbol){
                num = sscanf(thisLine->line, "%s %s %[^,], %s %s",symbolic,opcode, operand1, operand2, tmp);
                if(find_sym(symbolic,temp_symtab)==NULL){ //심볼이 없는 심볼이면 에러.
                    error_process(CANNOT_FIND_SYMBOLIC, (i+1)*5,thisLine->line);
                    return -1;
                }
                num-=1;
            }
            else num = sscanf(thisLine->line, "%s %[^,], %s %s",opcode, operand1, operand2, tmp);
            //라인을 읽는다.
            unsigned int temp;
            int nbit = 1, ibit = 1;
            int xbit = 0, bbit = 0, pbit = 0, ebit = 0;
            int immediate = 0;
            int addr = 0;
            int t = -1;
            int pc = lines[i+1]->location_counter;
            symbol_element* to_get;
            int toGet;

            switch(thisLine->format){
                case type_1: //1형식
                    if(num!=1){
                        error_process(ELEMENT_NUMBER_ERROR, (i+1)*5,thisLine->line);//1개가 아니라면 에러다.
                        return -1;
                    }
                    x = get_code(opcode);//opcode를 기준으로 1바이트의 objectcode를 만들어준다.
                    thisLine->objectCode = make_object_code(x,2);
                    fprintf(lst_file,"%-6d %04X     %-30s %-30s\n",(i+1)*5, thisLine->location_counter, thisLine->line, thisLine->objectCode);
                    break;
                case type_2:
                    if(num!=3&&num!=2){//clear로 2개 또는 opcode reg1, reg2로 세개가 나올 수 있다.
                        error_process(ELEMENT_NUMBER_ERROR, (i+1)*5,thisLine->line);
                        return -1;
                    }
                    x = get_code(opcode);
                    op1 = regToNumber(operand1);
                    op2 = 0;
                    if(num==3) {
                        op2 = regToNumber(operand2);
                        if((!strcmp(opcode,"CLEAR")||!strcmp(opcode,"SVC")||!strcmp(opcode,"TIXR"))){
                            error_process(INVALID_OP_CODE,(i+1)*5,thisLine->line);
                            return -1;
                        }
                    }
                    if(op1==-1||op2==-1){
                        error_process(CANNOT_FIND_REGISTER,(i+1)*5,thisLine->line);
                        return -1;
                    }
                    if(num==2) {//2개라면 맨 왼쪽 halfbte는 0으로 해준다.
                        if(!(!strcmp(opcode,"CLEAR")||!strcmp(opcode,"SVC")||!strcmp(opcode,"TIXR"))){
                            error_process(INVALID_OP_CODE,(i+1)*5,thisLine->line);
                            return -1;
                        }
                        temp = (unsigned int)x*16*16;
                        temp += op1*16;
                        thisLine->objectCode = make_object_code(temp,4);
                    }
                    else {
                        x = x*16*16;
                        x += op1*16;
                        x += op2;//아니면 맨 왼쪽 halfbyte는 op2로 해준다.
                        thisLine->objectCode = make_object_code(x,4);
                    }
                    fprintf(lst_file,"%-6d %04X     %-30s %-30s\n",(i+1)*5, thisLine->location_counter, thisLine->line, thisLine->objectCode);
                    break;
                case type_3://type3 //한개, 두개, 세개가 올 수 있다.
                    if(num==1){//한개일때
                        x = get_code(opcode);
                        temp = (x+(nbit<<1)+(ibit))<<16;//맨 오른쪽 비트만 들어가고 출력된다.
                        thisLine->objectCode = make_object_code(temp,6);
                        fprintf(lst_file,"%-6d %04X     %-30s %-30s\n",(i+1)*5, thisLine->location_counter, thisLine->line, thisLine->objectCode);
                        continue;
                    }
                    if(num==4||num==0){//4개일때는 에러이다.
                        error_process(ELEMENT_NUMBER_ERROR, (i+1)*5,thisLine->line);
                        return -1;
                    }
                    //두개, 세개일 때에는 relative addressing mode를 사용해야하고, immediate인지, simple인지, indirect인지를 정해줘야한다.
                    if(operand1[0]=='#') nbit = 0;
                    if(operand1[0]=='@') ibit = 0;
                    //정해줬다.

                    if(num==3){ 
                        t = regToNumber(operand2);
                        if(t==1) xbit = 1;//xbit이 set되었는가? 를 확인해본다.
                        else{
                            if(t==-1) error_process(CANNOT_FIND_REGISTER,(i+1)*5,thisLine->line);
                            else error_process(NO_X_BIT,(i+1)*5,thisLine->line);
                            return -1;
                        }
                    }
                    if(nbit==0||ibit==0)
                        to_get = find_sym(operand1+1,temp_symtab); 
                    else
                        to_get = find_sym(operand1,temp_symtab);
                    if(to_get==NULL){
                        
                        if(nbit==0&&ibit==1){//옆에 #이 왔을 경우
                            char* iter = operand1+1;

                            while(*iter!='\0'){
                                if(*iter<'0'||*iter>'9') break;
                                iter++;
                            }//이놈이 숫자이면
                            if(*iter=='\0'){
                                toGet = atoi(operand1+1);
                                addr = toGet;
                                immediate = 1;//그것은 상수다.
                            }
                            else {error_process(CANNOT_FIND_SYMBOLIC,(i+1)*5,thisLine->line); return -1;}//그 외의 경우는 에러
                        }
                        else {error_process(CANNOT_FIND_SYMBOLIC,(i+1)*5,thisLine->line); return -1;}
                    }
                    
                    if(to_get!=NULL)
                        toGet = to_get->location_counter;
                    
                    if(!immediate){//상수항은 이미 addr이 계산되어 있다.
                        if(-2048<=toGet-pc&&toGet-pc<=2047){ //1. pc relative가 가능한가?
                            pbit = 1;
                            addr = toGet-pc;
                            if(addr<0){
                                addr += (1<<12);
                            }
                        }
                        else if(baseValue!=-1&&toGet-baseValue>=0&&toGet-baseValue<=4095){//2. base relative가 가능한가?
                            bbit = 1;
                            addr = toGet-baseValue;
                        }
                        else{//다 안되면 3형식으로는 안되는 놈임
                            error_process(CANNOT_RELATIVE_ADDR,(i+1)*5,thisLine->line);
                            return -1;
                        }
                    }
                    x = get_code(opcode);
                    
                    temp = (x+(nbit<<1)+(ibit))<<16;
                    temp += ((xbit<<3)+(bbit<<2)+(pbit<<1)+ebit)<<12;
                    temp += addr;
                    thisLine->objectCode = make_object_code(temp,6);
                    fprintf(lst_file,"%-6d %04X     %-30s %-30s\n",(i+1)*5, thisLine->location_counter, thisLine->line, thisLine->objectCode); //형식에 맞춰서 잘 계산해서 넣어준다.
                    break;
                case type_4:
                    if(operand1[0]=='#') nbit = 0;
                    if(operand1[0]=='@') ibit = 0;
                    

                    if(num==3){ 
                        t = regToNumber(operand2);
                        if(t==1) xbit = 1;//xbit이 set되었는가? 를 확인해본다.
                        else{
                            error_process(CANNOT_FIND_REGISTER,(i+1)*5,thisLine->line);
                            return -1;
                        }
                    }
                    thisLine->need_modification_record = 1;
                    if(nbit==0||ibit==0)
                        to_get = find_sym(operand1+1,temp_symtab); 
                    else
                        to_get = find_sym(operand1,temp_symtab);
                    if(to_get==NULL){         
                        if(nbit==0&&ibit==1){
                            char* iter = operand1+1;

                            while(*iter!='\0'){
                                if(*iter<'0'||*iter>'9') break;
                                iter++;
                            }
                            if(*iter=='\0'){
                                toGet = atoi(operand1+1);
                                addr = toGet;
                                immediate = 1;
                                thisLine->need_modification_record = 0;
                            }
                            else {error_process(CANNOT_FIND_SYMBOLIC,(i+1)*5,thisLine->line);  return -1;}
                        }
                        else {error_process(CANNOT_FIND_SYMBOLIC,(i+1)*5,thisLine->line); 
                        return -1;}
                    }
                    ebit = 1; //3형식과 비슷하지만, extended bit이 설정되고, pc, b bit이 0이다.
                    if(to_get!=NULL)
                        toGet = to_get->location_counter;
                    addr = toGet;
                    x = get_code(opcode+1);
                    temp = (x+(nbit<<1)+(ibit))<<24;
                    temp += ((xbit<<3)+(bbit<<2)+(pbit<<1)+ebit)<<20;
                    temp += addr;
                    thisLine->objectCode = make_object_code(temp,8);
                    fprintf(lst_file,"%-6d %04X     %-30s %-30s\n",(i+1)*5, thisLine->location_counter, thisLine->line, thisLine->objectCode);
                    break;
            }
        }
    }
    //다 끝냈다면, object file을 만들러 간다.
    return make_object_file(obj_filename);
}
/**********************************
함수 : int make_object_file()
목적 : 오브젝트 파일을 만들어준다.
return : 에러가 있다면 -1, 에러가 없다면 0을 return
************************************/
int make_object_file(){
    obj_file = fopen(obj_filename,"w");
    char buf[150]={0};
    char buf2[150] = {0};
    int idx = 0;

    if(lines[0]->assembler_directives==START){
        sscanf(lines[0]->line,"%s",buf);
        fprintf(obj_file,"H%-6s%06X%06X\n",buf,lines[0]->location_counter,lines[lineNum-1]->location_counter-lines[0]->location_counter);
        idx++;
    }//첫줄에 start가 온 경우.
    int startAddr = -1;
    int max_buf_len = 60; //최대 출력할 수 있는 column (10~69)
    int cur_buf_len = 0; //현재 버퍼엔 몇개가 들어있는가
    for(int i = idx;i<lineNum-1;i++){
        if(lines[i]->assembler_directives==BASE) continue;
        if(lines[i]->assembler_directives==NOBASE) continue; //obj 파일엔 필요 없는 친구
        if(lines[i]->format==comments) continue; //obj 파일엔 필요 없는 친구
        if(lines[i]->objectCode==NULL){ //데이터 영역, 이 친구들의 메모리 영역을 남겨둬야하기 때문에 버퍼를 강제로 비워준다.
            if(cur_buf_len!=0) fprintf(obj_file,"%02X%s\n",cur_buf_len/2,buf);
            cur_buf_len = 0;
            continue;
        }
        if(startAddr==-1) startAddr = lines[i]->location_counter;
        if(cur_buf_len+strlen(lines[i]->objectCode)>=max_buf_len){ //이번에 넣으면 max_buf_len 보다 커지면 버퍼를 비워준다.
            fprintf(obj_file,"%02X%s\n",cur_buf_len/2,buf);
            cur_buf_len = 0;
        }

        if(cur_buf_len==0){//어떤 이유로 버퍼가 비워졌다면, 줄을 새로 시작해야한다.
            fprintf(obj_file,"T%06X",lines[i]->location_counter);
        }
        int templen = strlen(lines[i]->objectCode);
        for(int j = 0;j<templen;j++){//버퍼에 집어 넣는다.
            buf[cur_buf_len++] = lines[i]->objectCode[j];
        }
        buf[cur_buf_len] = 0;
    }
    if(cur_buf_len!=0){//마지막에 버퍼를 비워준다.
        fprintf(obj_file,"%02X%s\n",cur_buf_len/2,buf);
    }
    for(int i = idx;i<lineNum-1;i++){//modification record를 위한 구문.
        if(lines[i]->need_modification_record){
            fprintf(obj_file,"M%06X05\n",lines[i]->location_counter+1);
        }
    }
    int  n = sscanf(lines[lineNum-1]->line,"%s %s",buf,buf2);
    if(n==2){
        fprintf(obj_file,"E%06X\n",find_sym(buf2,temp_symtab)->location_counter);//end를 출력해준다.
    }
    else fprintf(obj_file,"E\n");
    fflush(obj_file);

    delete_symtab(symtab);//할당했던것을 해제해주자.
    for(int i= 0;i<97;i++){
        symtab[i] = temp_symtab[i]; //temp_symtab이 symtab으로 신분이 상승했다.
        temp_symtab[i] = NULL;
    }
    printf("([%s], [%s])\n",lst_filename,obj_filename);
    
    free(lst_filename); lst_filename = NULL;
    free(obj_filename); obj_filename = NULL;
    if(lst_file){fclose(lst_file); lst_file = NULL;}
    if(obj_file){fclose(obj_file); obj_file = NULL;}
    delete_line();
    return 0;
}

 
