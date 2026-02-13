#ifndef STACK_NODE_H
#define STACK_NODE_H

typedef struct StackNode {
  void *data;
  struct StackNode *next;
} StackNode;

#endif
