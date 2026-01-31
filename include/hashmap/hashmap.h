#ifndef HASHMAP_H
#define HASHMAP_H

#include "hashmap/bucket.h"

typedef struct HashMap {
    int global_depth;
    int bucket_limit;
    int dir_size;
    Bucket **directory;
} HashMap;

HashMap *createHashMap(int bucket_limit);
void insertHashMap(HashMap *map, Symbol *symbol);
Symbol *findHashMap(HashMap *map, const char *lexeme);
void deleteHashMap(HashMap *map);

#endif
