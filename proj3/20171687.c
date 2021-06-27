//20171687.c

#include "my_header.h"

int main(){     
    hstry = (history*)calloc(1,sizeof(history)); //프로그램 시작 전, hstry를 초기화해준다.
    init_hash_table(); //프로그램 시작 전, hash table을 읽어들여온다.
    do{
        char current_instruction[COMMAND_LENGTH]; //입력을 받는다.
        printf("sicsim> ");
        fgets(current_instruction,COMMAND_LENGTH,stdin);
        if(!manufacture_instruction(current_instruction)){//만약 invalid한 instruction이 들어왔다면
            printf("invalid instruction\n");//에러 메세지를 출력한다.
        }
    }while(!is_quit);//quit이 되었다면 프로그램을 정상적으로 종료한다.
    return 0;
}