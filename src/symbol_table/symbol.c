#include <stdlib.h>
#include <string.h>

#include "symbol_table/symbol.h"

Symbol *getSymbol(char *lexeme, int size, char *type, char *scope) {
  Symbol *sym = malloc(sizeof(Symbol));

  strcpy(sym->lexeme, lexeme);
  strcpy(sym->type, type);
  strcpy(sym->scope, scope);
  sym->size = size;

  return sym;
}
