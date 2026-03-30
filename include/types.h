#ifndef TYPES_H
#define TYPES_H

/**
 * @file types.h
 * @brief Common type definitions used throughout ova-lib.
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Comparator function type for ordering elements.
 *
 * Returns a negative value if @p a < @p b, zero if equal, or a positive value if @p a > @p b.
 */
typedef int (*comparator)(const void *a, const void *b);

/**
 * @brief Hash function type for mapping keys to bucket indices.
 *
 * @param key Pointer to the key to hash.
 * @param capacity Number of buckets.
 * @return A bucket index in the range [0, capacity).
 */
typedef int (*hash_func_t)(void *key, int capacity);

#endif /* TYPES_H */
