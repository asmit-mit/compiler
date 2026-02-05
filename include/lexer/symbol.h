#ifndef SYMBOL_H
#define SYMBOL_H

typedef struct Symbol {
    char lexeme[20];
    int size;
    char type[20];
    char scope[20];
} Symbol;

Symbol *symbol_create(char lexeme[], int size, char type[], char scope[]);
int symbol_compare(const Symbol *a, const Symbol *b);
int symbol_getIndex(const Symbol *sym, int depth);

#endif
