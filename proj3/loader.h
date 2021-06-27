#ifndef __LOADER_H__
#define __LOADER_H__
#include "my_header.h"
int progaddr;
int set_progaddr(char* address);

typedef struct ESTAB_TYPE{
    char name[7];
    unsigned int addr;
    int is_control_section;
    int length;
}ESTAB_TYPE;

typedef struct ET{
    ESTAB_TYPE* ET;
    int size;
}ET;

int total_length;
void push(ET* ESTAB,char* name, unsigned int addr, int is_control_section, int length);

enum CONTROL_SECTION{
    not_control_section, control_section
};

void print_ET(ET ESTAB);
int linking_loader_pass1(char filename[][COMMAND_LENGTH],int fileNum);
int linking_loader(char filename[][COMMAND_LENGTH],int fileNum);

#endif