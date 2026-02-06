#ifndef LEXER_H
#define LEXER_H

#include <ctype.h>
#include <stdio.h>

#include "lexer/token.h"

char nextChar(FILE *fp, int *row, int *col);
char ungetChar(char c, FILE *fp, int *row, int *col);

Token *isPunctuation(FILE *fp, int *row, int *col);
Token *isIdentifierOrKeyword(FILE *fp, int *row, int *col, int *index);
Token *isKeyword(FILE *fp, int *row, int *col);
Token *isNum(FILE *fp, int *row, int *col);
Token *isAssignop(FILE *fp, int *row, int *col);
Token *isMulop(FILE *fp, int *row, int *col);
Token *isAddop(FILE *fp, int *row, int *col);
Token *isLogicalop(FILE *fp, int *row, int *col);
Token *isRelop(FILE *fp, int *row, int *col);

Token *getNextToken(FILE *fp);

#endif
