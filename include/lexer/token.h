#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>
#include <string.h>

typedef struct Token {
  char token_name[100];
  int index;
  unsigned int row, col;
  char type[100];
} Token;

Token *getToken(char token_name[], int row, int col, int index, char type[]);

#endif
