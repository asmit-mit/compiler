#ifndef STACK_H
#define STACK_H

#include "stack_node.h"

typedef struct Stack {
  StackNode *head;
} Stack;

Stack *stack_create();
void stack_push(Stack *stk, void *data);
void *stack_top(Stack *stk);
void stack_pop(Stack *stk);
void stack_destroy(Stack *stk);

#endif
