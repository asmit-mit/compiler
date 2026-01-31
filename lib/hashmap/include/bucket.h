#ifndef BUCKET_H
#define BUCKET_H

#include "entry.h"

typedef struct Bucket {
    int local_depth;
    int size;
    Entry *head;
} Bucket;

Bucket *createBucket(int local_depth);
void insertBucket(Bucket *bucket, void *data);
void clearBucket(Bucket *bucket);

#endif
