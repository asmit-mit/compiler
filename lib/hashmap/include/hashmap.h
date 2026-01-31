#ifndef HASHMAP_H
#define HASHMAP_H

#include "bucket.h"

typedef struct HashMap {
  int global_depth;
  int bucket_limit;
  int dir_size;
  Bucket **directory;

  int (*getIndex)(const void *data, int depth);
  int (*comparator)(const void *a, const void *b);
} HashMap;

HashMap *createHashMap(int bucket_limit, void *getIndexFunction, void *comparator);
void insertHashMap(HashMap *map, void *data);
void *findHashMap(HashMap *map, void *data);
void destroyHashMap(HashMap *map);

#endif
