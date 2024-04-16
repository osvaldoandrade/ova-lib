#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stddef.h>
#include <stdbool.h>

typedef int (*comparator)(const void *a, const void *b);

typedef int (*hash_func_t)(void *key, int capacity);

#endif /* TYPES_H */
