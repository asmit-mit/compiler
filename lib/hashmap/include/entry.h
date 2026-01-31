#ifndef ENTRY_H
#define ENTRY_H

typedef struct Entry {
    void *data;
    struct Entry *next;
} Entry;

#endif
