#ifndef __OPCODE_H__
#define __OPCODE_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
typedef struct OPCODE{
    int code;
    char format[100];
    char mnemoinc[100];
    struct OPCODE* next;
}opcode;

typedef struct hashNode{
    opcode* head,*tail;
}hashNode;

hashNode* hashTable[20];

int hash(const char* code);
bool init_hash_table();
void push_to_hash(opcode* cur_opcode);
void opcodelist();
bool mnemonic(const char* tofind);
void free_hash_table();

int get_code(const char* tofind);
char* get_format(const char* tofind);
char* opcode_to_format(int code);
#endif