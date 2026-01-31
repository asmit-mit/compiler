#ifndef BUCKET_H
#define BUCKET_H

#include "hashmap/entry.h"

typedef struct Bucket {
    int local_depth;
    int size;
    Entry *head;
} Bucket;

Bucket *createBucket(int local_depth);
void insertBucket(Bucket *bucket, Symbol *symbol);
void clearBucket(Bucket *bucket);

#endif
