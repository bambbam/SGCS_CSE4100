#ifndef __ASSEMBLER_H__
#define __ASSEMBLER_H__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
typedef struct symbol_element{
    int location_counter;
    char* symbol;
    struct symbol_element* next;
}symbol_element;

symbol_element* symtab[97];
symbol_element* temp_symtab[97];

extern FILE* asm_file, *lst_file, *obj_file;

typedef struct{
    char* line;
    int location_counter;
    int isSymbol;
    int format; //0이면 comment
    int assembler_directives;
    char* objectCode;
    int need_modification_record;
}lineTab;


enum formats{
    comments=0,type_1,type_2,type_3,type_4,asm_dir
};
enum assembler_directives{
    nothing=0,START,END,BYTE,WORD,RESB,RESW,BASE,NOBASE
};
void error_process(int error_type, int lineNUmber, const char* line);
enum error_type{
    ELEMENT_NUMBER_ERROR, 
    CANNOT_FIND_SYMBOLIC,  
    CANNOT_FIND_REGISTER, 
    CANNOT_RELATIVE_ADDR, 
    SAME_SYMBOL_ERROR, 
    INVALID_OP_CODE,
    SAME_OPCODE_SYMBOL,
    NO_X_BIT,
    CANNOT_HEX_IN_START,
    INVALID_OPERAND
};

int is_valid_Number(char* num, int upper);
int hash_sym(const char* todo);
int push_symtab(const char* todo, int lc, symbol_element* symtab[97]);
void delete_symtab(symbol_element* symtab[97]);
int print_symtab(symbol_element* symtab[97]);

void delete_line();
void push_line(char* buf, int location_counter, int is_symbolic, int format, int assembelr_directives);
int assemble(const char* filename);
symbol_element* find_sym(const char* x, symbol_element*[97]); //찾으면 return0 못찾으면 return -1
int regToNumber(const char* a);
char* make_object_code(unsigned int x, int operand_length);
int path_two(const char* filename);
int make_object_file();





extern char* lst_filename;
extern char* obj_filename;

extern lineTab** lines;
extern int lineNum;

#endif