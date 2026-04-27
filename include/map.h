#ifndef MAP_H
#define MAP_H

#include "types.h"

#define HASH_FUNC_COUNT 5
#define INITIAL_CAPACITY 20
#define LOAD_FACTOR 0.75

typedef enum {
    HASH_TABLE,
    HASH_MAP
} map_type;

/**
 * @brief Public map object.
 *
 * Concrete storage details live in @p impl.
 */
typedef struct map {
    void *impl;

    /**
     * @brief Insert or update a key/value pair.
     *
     * @param self Map instance.
     * @param key Key pointer.
     * @param data Value pointer.
     * @return OVA_SUCCESS on success, or a negative ova_error_code on failure.
     */
    ova_error_code (*put)(struct map *self, void *key, void *data);

    /**
     * @brief Insert multiple key/value pairs.
     *
     * @param self Map instance.
     * @param keys Array of key pointers.
     * @param values Array of value pointers.
     * @param count Number of pairs to insert.
     * @return OVA_SUCCESS on success, or a negative ova_error_code on failure.
     */
    ova_error_code (*put_bulk)(struct map *self, void **keys, void **values, int count);

    /**
     * @brief Retrieve the value associated with a key.
     *
     * @param self Map instance.
     * @param key Key pointer.
     * @return Stored value pointer, or NULL when missing.
     */
    void *(*get)(struct map *self, void *key);

    /**
     * @brief Remove and return the value associated with a key.
     *
     * @param self Map instance.
     * @param key Key pointer.
     * @return Removed value pointer, or NULL when missing.
     */
    void *(*remove)(struct map *self, void *key);

    /**
     * @brief Return the number of stored key/value pairs.
     *
     * @param self Map instance.
     * @return Number of stored pairs.
     */
    int (*size)(const struct map *self);

    /**
     * @brief Return the current bucket capacity.
     *
     * @param self Map instance.
     * @return Number of buckets currently allocated.
     */
    int (*capacity)(const struct map *self);

    /**
     * @brief Remove all key/value pairs without destroying the map.
     *
     * The map becomes empty but retains its bucket capacity.
     * User keys and values are not freed.
     *
     * @param self Map instance.
     */
    void (*clear)(struct map *self);

    /**
     * @brief Release the map and its internal allocations.
     *
     * The map does not free user keys or values.
     *
     * @param self Map instance.
     */
    void (*free)(struct map *self);
} map;

/**
 * @brief Create a new map.
 *
 * @param type Map backend to construct.
 * @param capacity Initial capacity hint.
 * @param hash_func Hash function used to place keys into buckets.
 * @param compare Comparator used to test keys for equality.
 * @return New map instance, or NULL on failure.
 */
map *create_map(map_type type, int capacity, int (*hash_func)(void *, int), comparator compare);

int bernstein_hash(void *key, int capacity);
int fnv1a_hash(void *key, int capacity);
int xor_hash(void *key, int capacity);
int rotational_hash(void *key, int capacity);
int additive_hash(void *key, int capacity);

#endif // MAP_H
