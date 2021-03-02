#ifndef STACK_H
#define STACK_H

#include <stdlib.h>

typedef struct Node
{
    void * _data; // pointer to anything
    struct Node * _next;
} Node;

typedef struct Stack
{
    Node * _top;
    size_t _elementSize;    // we need that for memcpy
    size_t _stackSize;
} Stack;

Stack* stackAlloc(size_t elementSize);

void freeStack(Stack** stack);

void push(Stack* stack, void *data);

void pop(Stack* stack,void *headData);

int isEmptyStack(Stack* stack);

#endif
