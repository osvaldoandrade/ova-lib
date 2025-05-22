/**
 * @file types.h
 * @brief Common types used across the library.
 */

#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef int (*comparator)(const void *a, const void *b);

typedef int (*hash_func_t)(void *key, int capacity);

#endif /* TYPES_H */
