CC = gcc
CFLAGS = -Wall
TARGET = 20171687.out

$(TARGET) : my_header.o 20171687.o memory.o opcode.o assembler.o loader.o run.o breakpoint.o
	$(CC) $(CFLAGS) -o $(TARGET) my_header.o 20171687.o memory.o opcode.o assembler.o loader.o run.o breakpoint.o

20171687.o : 20171687.c 
	$(CC) $(CFLAGS) -c -o 20171687.o 20171687.c

my_header.o : my_header.c
	$(CC) $(CFLAGS) -c -o my_header.o my_header.c

memory.o : memory.c
	$(CC) $(CFLAGS) -c -o memory.o memory.c

opcode.o : opcode.c
	$(CC) $(CFLAGS) -c -o opcode.o opcode.c

assembler.o : assembler.c
	$(CC) $(CFLAGS) -c -o assembler.o assembler.c

loader.o : loader.c
	$(CC) $(CFLAGS) -c -o loader.o loader.c

run.o : run.c
	$(CC) $(CFLAGS) -c -o run.o run.c
breakpoint.o : breakpoint.c
	$(CC) $(CFLAGS) -c -o breakpoint.o breakpoint.c

clean :
	rm *.o 20171687.out 
