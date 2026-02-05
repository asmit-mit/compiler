#ifndef BUCKET_H
#define BUCKET_H

#include "entry.h"

typedef struct Bucket {
    int local_depth;
    int size;
    Entry *head;
} Bucket;

Bucket *bucket_create(int local_depth);
void bucket_insert(Bucket *bucket, void *data);
void bucket_clear(Bucket *bucket);

#endif
