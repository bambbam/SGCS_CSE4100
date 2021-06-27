#include "memory.h"

//x와 y 중 더 작은 값을 return한다.
int MIN(int x, int y){
    return x<y?x:y;
}
/**********************************
함수 : dump(int start, int end, int mode)
목적 : 
mode 0 : start,end가 정해지지 않은 것. start->MEMORY_IDX, end -> MIN(start + 159, MEMORY_SIZE - 1)
mode 1 : start만 정해진것 start->MEMORY_IDX, end-> MIN(start + 159, MEMORY_SIZE - 1), start가 범위를 벗어난다면 error
mode 2 : start,end가 정해진것. start, end가 범위를 벗어난다면 error

start~end까지의 메모리내에 있는 값들을 출력한다.

return : 
start,end의 범위가 정상이라면 true,
start,end의 범위가 정상이 아니라면 false
************************************/
bool dump(int start, int end, int mode) {
    if (mode == 0) start = MEMORY_IDX;
    if (mode != 2) end = MIN(start + 159, MEMORY_SIZE - 1);
    if ((start > end || start < 0 || start >= MEMORY_SIZE || end < 0 || end >= MEMORY_SIZE)) {
        return false;
    }
    printf("%05X ", start - start%16);
    for (int i = (start - start%16); i < start; i++) {
        printf("   ");
    }

    for (int i = start; i <= end; i++) {
        if (i != start && !(i % 16)) {
            printf("; ");
            for (int j = i - 16; j < i; j++) {
                if (j>=start&&j<=end&&memory[j] <= 0x7E && memory[j] >= 0x20) {
                    printf("%c", memory[j]);
                }   
                else printf(".");
            }
            printf("\n%05X ", i);
        }
        printf("%02X ", memory[i]);
    }
    for (int i = end + 1; i < (end - end%16 + 16); i++) {
        printf("   ");
    }
    printf("; ");
    for (int j = end - (end % 16); j < (end - end % 16 + 16); j++) {
        if (j>=start&&j<=end&&memory[j] <= 0x7E && memory[j] >= 0x20) {
            printf("%c", memory[j]);
        }
        else printf(".");
    }
    printf("\n");
    MEMORY_IDX = end+1;
    if(MEMORY_IDX>=MEMORY_SIZE) MEMORY_IDX = 0;
    return true;
}
/**********************************
함수 : edit(int address, int value)
목적 : 
address에 해당하는 메모리의 값을 value로 바꾼다.
address가 범위를 벗어나거나, value가 범위를 벗어나면 error
return : 
address, value의 범위가 정상이라면 true
아니라면 false
************************************/
bool edit(int address, int value){
    if(address<0||address>=MEMORY_SIZE) return false;
    if(value>0xFF||value<0) return false;
    memory[address] = value;
    return true;
}
/**********************************
함수 : fill(int start, int end, int value)
목적 : start~end의 값을 value로 바꿔준다.
start>end거나, start, end가 허용 범위를 벗어나거나
value가 허용범위를 벗어나면 error

return : 
start,end value의 범위가 정상이라면 true
아니라면 false
************************************/
bool fill(int start, int end, int value){
    if(start>end||start<0||end<0||start>=MEMORY_SIZE||end>=MEMORY_SIZE) return false;
    if(value>0xFF||value<0) return false;
    for(int i = start;i<=end;i++){
        memory[i] = value;
    }
    return true;
}
/**********************************
함수 : reset()
목적 : 메모리의 값을 0으로 reset해준다.
return : 없음
************************************/

bool reset(){
    for(int i = 0;i<MEMORY_SIZE;i++){
        memory[i] = 0;
    }
    return true;
}

