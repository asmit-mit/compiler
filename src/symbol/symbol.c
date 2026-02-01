#include <stdlib.h>
#include <string.h>

#include "symbol/symbol.h"

Symbol *getSymbol(char *lexeme, int size, char *type, char *scope) {
  Symbol *sym = malloc(sizeof(Symbol));

  strcpy(sym->lexeme, lexeme);
  strcpy(sym->type, type);
  strcpy(sym->scope, scope);
  sym->size = size;

  return sym;
}

int compareSymbol(const Symbol *a, const Symbol *b) {
  return strcmp(a->lexeme, b->lexeme) == 0;
}

int getIndex(const Symbol *sym, int depth) {
  unsigned hash = 5381;
  const char *lexeme = sym->lexeme;
  while (*lexeme)
    hash = ((hash << 5) + hash) + *lexeme++;
  return hash & ((1 << depth) - 1);
}
