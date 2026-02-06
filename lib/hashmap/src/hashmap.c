#include <stdlib.h>
#include <string.h>

#include "hashmap.h"

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

  Bucket *new_bucket = bucket_create(old->local_depth + 1);
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
    Entry *next = curr->next;
    int idx = map->getIndex(curr->data, map->global_depth);

    bucket_insert(map->directory[idx], curr->data);
    free(curr);

    curr = next;
  }
}

HashMap *hashmap_create(int bucket_limit, void *getIndexFunction, void *comparator) {
  HashMap *map = malloc(sizeof(HashMap));
  if (!map)
    return NULL;

  map->global_depth = 1;
  map->bucket_limit = bucket_limit;
  map->dir_size = 2;
  map->getIndex = getIndexFunction;
  map->comparator = comparator;

  map->directory = malloc(sizeof(Bucket *) * map->dir_size);

  Bucket *b0 = bucket_create(1);
  Bucket *b1 = bucket_create(1);

  map->directory[0] = b0;
  map->directory[1] = b1;

  return map;
}

void hashmap_insert(HashMap *map, void *data) {
  if (!map || !data)
    return;

  if (hashmap_find(map, data))
    return;

  int idx = map->getIndex(data, map->global_depth);
  Bucket *b = map->directory[idx];

  if (b->size < map->bucket_limit) {
    bucket_insert(b, data);
    return;
  }

  splitBucket(map, idx);
  hashmap_insert(map, data);
}

void *hashmap_find(HashMap *map, void *data) {
  if (!map || !data)
    return NULL;

  int idx = map->getIndex(data, map->global_depth);
  Bucket *b = map->directory[idx];

  for (Entry *e = b->head; e; e = e->next) {
    if (map->comparator(e->data, data))
      return e->data;
  }

  return NULL;
}

void hashmap_destroy(HashMap *map) {
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
      bucket_clear(map->directory[i]);
      free(map->directory[i]);
    }
  }

  free(map->directory);
  free(map);
}
