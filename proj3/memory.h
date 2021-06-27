#ifndef __MEMORY_H__
#define __MEMORY_H__
#define MEMORY_SIZE (1<<20)
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

int MIN(int x, int y);

unsigned char memory[MEMORY_SIZE];
int MEMORY_IDX;


bool dump(int start, int end, int mode);
bool edit(int address, int value);
bool fill(int start, int end, int value);
bool reset();

#endif