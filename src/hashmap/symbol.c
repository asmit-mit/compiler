#include <stdlib.h>
#include <string.h>

#include "hashmap/symbol.h"

Symbol *getSymbol(char lexeme[], int size, char type[], char scope[]) {
  Symbol *node = (Symbol *)malloc(sizeof(Symbol));
  strcpy(node->lexeme, lexeme);
  strcpy(node->type, type);
  strcpy(node->scope, scope);
  node->size = size;
  return node;
}
