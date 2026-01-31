#include <stdlib.h>
#include <string.h>

#include "hashmap/hashmap.h"

static unsigned hash(const char *s) {
  unsigned hash = 5381;
  while (*s)
    hash = ((hash << 5) + hash) + *s++;
  return hash;
}

static int getIndex(const char *lexeme, int depth) {
  return hash(lexeme) & ((1 << depth) - 1);
}

static void doubleDirectory(HashMap *map) {
  int old_size = map->dir_size;

  map->dir_size *= 2;
  map->global_depth++;

  map->directory = realloc(map->directory, sizeof(Bucket *) * map->dir_size);

  for (int i = 0; i < old_size; i++) {
    map->directory[i + old_size] = map->directory[i];
  }
}

static void splitBucket(HashMap *map, int dir_index) {
  Bucket *old = map->directory[dir_index];

  if (old->local_depth == map->global_depth) {
    doubleDirectory(map);
  }

  Bucket *new_bucket = createBucket(old->local_depth + 1);
  old->local_depth++;

  int bit = 1 << (old->local_depth - 1);

  for (int i = 0; i < map->dir_size; i++) {
    if (map->directory[i] == old && (i & bit)) {
      map->directory[i] = new_bucket;
    }
  }

  Entry *curr = old->head;
  old->head = NULL;
  old->size = 0;

  while (curr) {
    int idx = getIndex(curr->symbol->lexeme, map->global_depth);

    insertBucket(map->directory[idx], curr->symbol);
    free(curr);

    curr = curr->next;
  }
}

HashMap *createHashMap(int bucket_limit) {
  HashMap *map = malloc(sizeof(HashMap));
  if (!map)
    return NULL;

  map->global_depth = 1;
  map->bucket_limit = bucket_limit;
  map->dir_size = 2;

  map->directory = malloc(sizeof(Bucket *) * map->dir_size);

  Bucket *b0 = createBucket(1);
  Bucket *b1 = createBucket(1);

  map->directory[0] = b0;
  map->directory[1] = b1;

  return map;
}

void insertHashMap(HashMap *map, Symbol *symbol) {
  if (!map || !symbol)
    return;

  int idx = getIndex(symbol->lexeme, map->global_depth);
  Bucket *b = map->directory[idx];

  if (b->size < map->bucket_limit) {
    insertBucket(b, symbol);
    return;
  }

  splitBucket(map, idx);
  insertHashMap(map, symbol);
}

Symbol *findHashMap(HashMap *map, const char *lexeme) {
  if (!map || !lexeme)
    return NULL;

  int idx = getIndex(lexeme, map->global_depth);
  Bucket *b = map->directory[idx];

  for (Entry *e = b->head; e; e = e->next) {
    if (strcmp(e->symbol->lexeme, lexeme) == 0)
      return e->symbol;
  }

  return NULL;
}

void deleteHashMap(HashMap *map) {
  if (!map)
    return;

  int size = map->dir_size;

  for (int i = 0; i < size; i++) {
    int unique = 1;
    for (int j = 0; j < i; j++) {
      if (map->directory[i] == map->directory[j]) {
        unique = 0;
        break;
      }
    }

    if (unique) {
      clearBucket(map->directory[i]);
      free(map->directory[i]);
    }
  }

  free(map->directory);
  free(map);
}
