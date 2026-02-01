#include <stdlib.h>

typedef struct Entry {
  void *data;
  struct Entry *next;
} Entry;

typedef struct Bucket {
  int local_depth;
  int size;
  Entry *head;
} Bucket;

typedef struct HashMap {
  int global_depth;
  int bucket_limit;
  int dir_size;
  Bucket **directory;

  int (*getIndex)(const void *data, int depth);
  int (*comparator)(const void *a, const void *b);
} HashMap;

Bucket *createBucket(int local_depth) {
  Bucket *b = malloc(sizeof(Bucket));
  if (!b)
    return NULL;

  b->local_depth = local_depth;
  b->size = 0;
  b->head = NULL;
  return b;
}

void insertBucket(Bucket *bucket, void *data) {
  if (!bucket || !data)
    return;

  Entry *e = malloc(sizeof(Entry));
  if (!e)
    return;

  e->data = data;
  e->next = bucket->head;
  bucket->head = e;
  bucket->size++;
}

void clearBucket(Bucket *bucket) {
  if (!bucket)
    return;

  Entry *curr = bucket->head;
  while (curr) {
    Entry *next = curr->next;
    free(curr);
    // free(curr->data);
    curr = next;
  }

  bucket->head = NULL;
  bucket->size = 0;
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
    Entry *next = curr->next;

    int idx = map->getIndex(curr->data, map->global_depth);
    insertBucket(map->directory[idx], curr->data);

    free(curr);
    curr = next;
  }
}

HashMap *createHashMap(int bucket_limit, void *getIndexFunction,
                       void *comparator) {
  HashMap *map = malloc(sizeof(HashMap));
  if (!map)
    return NULL;

  map->global_depth = 1;
  map->bucket_limit = bucket_limit;
  map->dir_size = 2;
  map->getIndex = getIndexFunction;
  map->comparator = comparator;

  map->directory = malloc(sizeof(Bucket *) * map->dir_size);

  Bucket *b0 = createBucket(1);
  Bucket *b1 = createBucket(1);

  map->directory[0] = b0;
  map->directory[1] = b1;

  return map;
}

void *findHashMap(HashMap *map, void *data) {
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

void insertHashMap(HashMap *map, void *data) {
  if (!map || !data)
    return;

  if (findHashMap(map, data))
    return;

  while (1) {
    int idx = map->getIndex(data, map->global_depth);
    Bucket *b = map->directory[idx];

    if (b->size < map->bucket_limit) {
      insertBucket(b, data);
      return;
    }

    splitBucket(map, idx);
  }
}

void destroyHashMap(HashMap *map) {
  if (!map)
    return;

  for (int i = 0; i < map->dir_size; i++) {
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
