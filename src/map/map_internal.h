#ifndef MAP_INTERNAL_H
#define MAP_INTERNAL_H

#include "../../include/map.h"

#include <pthread.h>

typedef struct map_entry {
    void *key;
    void *data;
    struct map_entry *next;
} map_entry;

#define MAP_ENTRY_FREELIST_CAP 65536

typedef struct map_impl {
    map_type type;
    map_entry **buckets;
    int capacity;
    int size;
    int (*hash_func)(void *key, int capacity);
    comparator key_compare;
    pthread_mutex_t *lock;
    map_entry *free_head;
    int free_count;
} map_impl;

static inline map_impl *map_impl_from_map(const map *m) {
    return m ? (map_impl *)m->impl : NULL;
}

#endif // MAP_INTERNAL_H
