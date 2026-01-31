#ifndef SYMBOL_H
#define SYMBOL_H

typedef struct Symbol {
    char lexeme[20];
    int size;
    char type[20];
    char scope[20];
} Symbol;

Symbol *getSymbol(char lexeme[], int size, char type[], char scope[]);

#endif
