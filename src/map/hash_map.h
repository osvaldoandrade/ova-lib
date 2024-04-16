#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "../../include/map.h"

map *create_hash_map(int capacity, int (*hash_func)(void *, int), comparator key_compare, int thread_safe);

#endif // HASH_MAP_H
