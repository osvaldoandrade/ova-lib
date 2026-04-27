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

/**
 * @brief Consistent error codes returned by mutating operations.
 *
 * All insertion, deletion, and resize operations return one of these codes so
 * that callers can distinguish between different failure modes.
 */
typedef enum {
    OVA_SUCCESS               =  0,
    OVA_ERROR_MEMORY          = -1,
    OVA_ERROR_INVALID_ARG     = -2,
    OVA_ERROR_INDEX_OUT_OF_BOUNDS = -3,
    OVA_ERROR_EMPTY           = -4,
    OVA_ERROR_FULL            = -5,
    OVA_ERROR_NOT_FOUND       = -6
} ova_error_code;

#endif /* TYPES_H */
