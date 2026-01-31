#ifndef ENTRY_H
#define ENTRY_H

#include "hashmap/symbol.h"

typedef struct Entry {
    Symbol *symbol;
    struct Entry *next;
} Entry;

#endif
