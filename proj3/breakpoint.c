#ifndef __BREAKPOINT_H__
#define __BREAKPOINT_H__
#include "breakpoint.h"


void print_bp(){ //breakpoint를 출력한다.
    printf("\t\tbreakpoint\n");
    printf("\t\t----------\n");
    for(int i = 0;i<(1<<20);i++){
        if(bp_array[i]){
            printf("\t%X\n",i);    
        }
    }
}

int bp(char* address){ //break point를 set한다.
    if(!strcmp(address,"clear")){ //clear가 들어왔다면 clear를 해준다.
        printf("\t[\033[0;32mok\033[0;37m] clear all breakpoints\n");
        for(int i = 0;i<(1<<20);i++){
            bp_array[i] = 0;
        }
        return 0;
    }
    if(!is_valid_hex_number(address)) { //clear가 아닌데 16진수가 아니라면 예외다
        printf("error : invalid input\n");
        return -1;
    }
    
    int addr = convert_hex_to_int(address);
    if(bp_array[addr]){ //이미 들어왔던 입력이라면 이미 들어왔었다라고 알려준다.
        printf("this value is already set!\n");
        return -1;
    }
    bp_array[addr] = 1;
    printf("\t[\033[0;32mok\033[0;37m] create breakpoint %s\n",address);
    return 0;
}

int find_bp(int addr){ //이번 addr이 breakpoint가 set되어있는지를 알아본다.
    return bp_array[addr];
}

#endif