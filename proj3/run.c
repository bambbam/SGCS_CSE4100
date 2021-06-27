#include "run.h"
#include "opcode.h"
#include "loader.h"

const int twelv_bit = (1<<12)-1; //12비트 비트마스킹
const int twenty_bit = (1<<20)-1; //20비트 비트마스킹
const int four_bit = (1<<4)-1; //4비트 비트마스킹
int get_word(int addr){ //addr의 주소부터 3바이트에 해당하는 값을 읽어온다.
    return memory[addr+2] + (memory[addr+1]<<8) + (memory[addr]<<16);
}
int make_memory(int current_instruction,int addr,  int n, int i, int x, int b, int p, int e, int format){ //3,4형식일 때 addr를 계산한다.
    if(format==3){ 
        if(addr>=0x800){ //addr이 음수인경우
            addr ^= 0xFFF; //one's complement로 바꿔준다.
            addr += 1;
            addr *= -1;
        }
        if(p){
            addr += PC; // pc relative일 때
        }
        else if(b){ //base relative 일 때
            addr += B; 
        }
    }
    if(x){
        addr += X; //index bit이 set되어있을 때
    }
    if(n&&!i) {addr = get_word(addr);} //indirect addressing일 때
    return addr;
}

void print_register(){ //레지스터에 들어있는 값을 출력한다.
    printf("A : %06X  X : %06X\n",A,X);
    printf("L : %06X PC : %06X\n",L,PC);
    printf("B : %06X  S : %06X\n",B,S);
    printf("T : %06X\n",T);
}


void store_word(int addr, int reg){ //reg에 들어있는 값을 address에 옮긴다.
    memory[addr+2] = reg%256; //작은거부터
    reg/=256;
    memory[addr+1] = reg%256;
    reg/=256;
    memory[addr] = reg%256; //큰거로
}

int* get_register(int reg_num){ //레지스터 번호에 해당하는 register변수의 주소값을 넘긴다.
    switch(reg_num){
        case 0:
            return &A;
        case 1:
            return &X;
        case 2:
            return &L;
        case 3:
            return &B;
        case 4:
            return &S;
        case 5:
            return &T;
        case 8:
            return &PC;
        case 9:
            return &SW;
        default:
            return NULL;                                                                                    
    }
}

int run(){
    while(1){
        int current_instruction = PC; //현재의 instruction의 주소는 이전의 pc값이다.
        int opcode = memory[current_instruction]&((1<<8)-4); //opcode는 8비트중 상위 6비트의 값
        int format = 0; 
        int n,i,x,b,p,e;
        int addr = 0;
        int reg1, reg2;
        int* r1, *r2;
        n = i = x = b = p = e = 0; 
        char* f = opcode_to_format(opcode); //format을 구한다.
        if(!strcmp("1",f)) {PC+=1;format = 1;} 
        if(!strcmp("2",f)) {PC+=2;format = 2;}
        if(!strcmp("3/4",f)){
            if(memory[current_instruction+1]&(1<<4)){
                PC+=4;
                format =4;
            }   
            else {PC+=3; format = 3;} //형식에 맞춰 pc값을 더한다.
        }
        //printf("opcode : %02X, format : %d\n",opcode, format);
        if(format==1){ // format 이 1인경우는 이 프로그램에 존재하지 않는다.
            //pass;
        }
        else if(format==2){ //format이 2인경우
            switch(opcode){
                case 0xB4: //clear
                    reg1 = (memory[current_instruction+1]>>4)&four_bit; //상위 4비트를 가져온다.
                    r1 = get_register(reg1); //그 레지스터를 클리어시킨다.
                    *r1 = 0;
                    break;
                case 0xA0: //compr
                    reg2 = (memory[current_instruction+1])&four_bit; //하위 4비트를 가져온다
                    reg1 = (memory[current_instruction+1]>>4)&four_bit; //상위 4비트를 가져온다.
                    r1 = get_register(reg1); 
                    r2 = get_register(reg2);
                    SW = (*r1)-(*r2); //두 비트를 비교한다.
                    break;
                case 0xB8: //TIXR
                    reg1 = (memory[current_instruction+1]>>4)&four_bit; // 상위 4비트를 가져온다.
                    r1 = get_register(reg1);
                    X = X+1;
                    SW = X-(*r1); //X레지스터와 이번에 입력받은 레지스터를 비교한다.
                    break;
            }
        }
        else if(format==3||format==4){ //3,4형식일 때
            i = (memory[current_instruction]&1)!=0;
            n = (memory[current_instruction]&2)!=0;
            x = (memory[current_instruction+1]&(1<<7))!=0;
            b = (memory[current_instruction+1]&(1<<6))!=0;
            p = (memory[current_instruction+1]&(1<<5))!=0;
            e = (memory[current_instruction+1]&(1<<4))!=0; //n i x b p e 비트를 계산한다
           // printf("n : %d, i : %d, x : %d, b : %d, p : %d, e :  %d\n",n,i,x,b,p,e);
            if(format==3){
                addr = memory[current_instruction+2] + ((memory[current_instruction+1]<<8)&twelv_bit); //필요한 비트를 가져온다.
            }
            if(format==4){
                addr = memory[current_instruction+3] + (memory[current_instruction+2]<<8) + ((memory[current_instruction+1]<<16)&twenty_bit); //필요한 주소값을 가져온다.
            }
            addr = make_memory(current_instruction,addr,n,i,x,b,p,e,format); //가져온 주소값을 바탕으로 실제로 참조할 메모리를 계산한다.
            switch (opcode){
                case 0x14: //STL
                    store_word(addr,L);
                    break;
                case 0x68: //LDB
                    if(n) B = get_word(addr);
                    else B = addr;
                    break;
                case 0x48: //JSUB
                    L = PC;
                    PC = addr;
                    break;
                case 0x74: //LDT
                    if(n) T = get_word(addr);
                    else T = addr;
                    break;
                case 0xE0: //test device
                    SW = -1; 
                    break;
                case 0x30: //JEQ
                    if(SW==0){
                        PC = addr;
                    }
                    break;
                case 0xD8: //read device , 아무것도 못받았다고 가정
                    A = 0;
                    break;
                case 0x10: //STX
                    store_word(addr,X);
                    break;
                case 0x4C: //RSUB
                    PC = L;
                    break;
                case 0x00: //LDA
                    if(n) A = get_word(addr);
                    else A = addr;
                    break;
                case 0x28: //comp
                    if(n) SW = A - get_word(addr);
                    else SW = A-addr;
                    break;
                case 0x0C: //sta
                    store_word(addr,A);
                    break;
                case 0x50: //LDCH
                    if(n) A = memory[addr];
                    else A = (addr);
                    break;
                case 0x38: //JLT
                    if(SW<0){
                        PC = addr;
                    }
                    break;
                case 0x3C : //J
                    PC = addr;
                    break;
                default:
                    break;
             }
        }
        if(total_length+progaddr<=PC){ //프로그램이 끝났을 때
            print_register();
            printf("           End Program\n");
            A = X = B = S = T = SW = 0; PC = progaddr; L = total_length;
            break;
        }
        if(find_bp(PC)){ // break point를 만났을 때
            print_register();
            printf("           Stop at checkpoint[%X]\n",PC);
            break;
        }
        fflush(stdout);
    }
    return 0;
}