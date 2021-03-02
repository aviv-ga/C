OBJS = stack.o
CC = gcc
CFLAG = -c
FLAGS = -Wextra -Wall -Wvla -std=c99 -lm

all : $(OBJS)
	$(CC) $(FLAGS) $(OBJS) calc.c -o calc

stack.o: stack.h stack.c
	$(CC) $(FLAGS) $(CFLAG) stack.c -o stack.o

tar:
	tar cvf ex3.tar Makefile calc.c stack.h stack.c

clean :
	\rm -f *.o 
.PHONY : clean


