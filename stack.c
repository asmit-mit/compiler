#pragma once

#include <stdlib.h>

typedef struct StackNode {
  void *data;
  struct StackNode *next;
} StackNode;

typedef struct Stack {
  StackNode *head;
} Stack;

Stack *stack_create() {
  Stack *stk = (Stack *)malloc(sizeof(Stack));
  stk->head = NULL;
  return stk;
}

void stack_push(Stack *stk, void *data) {
  if (!stk || !data)
    return;

  StackNode *node = (StackNode *)malloc(sizeof(StackNode));
  node->data = data;
  node->next = stk->head;
  stk->head = node;
}

void *stack_top(Stack *stk) {
  if (!stk || !stk->head)
    return NULL;

  return stk->head->data;
}

void stack_pop(Stack *stk) {
  if (!stk || !stk->head)
    return;

  StackNode *temp = stk->head;
  stk->head = temp->next;

  free(temp->data);
  free(temp);
}

void stack_destroy(Stack *stk) {
  if (!stk)
    return;

  StackNode *curr = stk->head;
  while (curr) {
    StackNode *next = curr->next;

    free(curr->data);
    free(curr);

    curr = next;
  }

  free(stk);
}
