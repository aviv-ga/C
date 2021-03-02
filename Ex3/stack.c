#include "stack.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

Stack* stackAlloc(size_t elementSize)
{
  Stack* stack = (Stack*)malloc(sizeof(Stack));
  if(stack == NULL)
  {
      return NULL; // mem fault
  }
  stack->_top = NULL;
  stack->_elementSize = elementSize;
  stack->_stackSize = 0;
  return stack;
}

void freeStack(Stack** stack)
{
  Node* p1;
  Node* p2;
  if (!(*stack == NULL))
    {
      p1= (*stack)->_top;
      while(p1)
	{
	  p2= p1;
	  p1= p1->_next;
	  free(p2->_data);
	  free(p2);
	}
      free(*stack);
      *stack = NULL;
    }
}

void push(Stack* stack, void *data)
{
  Node* node = (Node*)malloc(sizeof(Node));
  if(node == NULL)
  {
      return; // memFault
  }
  node->_data = malloc(stack->_elementSize);
  if(node->_data == NULL)
  {
      return; // mem fault
  }
  memcpy(node->_data, data, stack->_elementSize);
  node->_next = stack->_top;
  stack->_top = node;
  stack->_stackSize++;
}

void pop(Stack* stack, void *headData) 
{
  assert(stack != NULL);
  if(stack->_top == NULL)
    {
      fprintf(stderr, "The stack is empty\n");
      return;
    }

  Node *node = stack->_top;
  memcpy(headData, node->_data,stack->_elementSize);
  stack->_top = node->_next;
  stack->_stackSize--;
  free(node->_data);
  free(node);
}

int isEmptyStack(Stack* stack) 
{
  assert(stack != NULL); 
  return stack->_top == NULL; 
}
