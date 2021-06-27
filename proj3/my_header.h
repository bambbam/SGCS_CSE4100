#ifndef __MY_HEADER_H__
#define __MY_HEADER_H__

#define COMMAND_LENGTH 100

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>
#include "memory.h"
#include "opcode.h"
#include "assembler.h"
#include "loader.h"
#include "run.h"
#include "breakpoint.h"
bool is_quit;

bool is_valid_hex_number(char*);
int convert_hex_to_int(char*);


void help();
void dir();
void quit();

bool manufacture_instruction(char[COMMAND_LENGTH]);

typedef struct HISTORY_NODE{
    int idx;
    int num_element;
    char* values[5];
    struct HISTORY_NODE* next;
}history_node;

typedef struct HISTORY{
    history_node* head;
    history_node* tail;
}history;
history* hstry;

void push_history_node(history*, char[5][COMMAND_LENGTH], int);
void print_history(history*);
void free_hstry();
int type(const char* filename);


int A,X,L,PC,B,S,T,SW;

#endif