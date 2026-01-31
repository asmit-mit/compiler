#include <stdlib.h>

#include "bucket.h"

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
    curr = next;
  }

  bucket->head = NULL;
  bucket->size = 0;
}
