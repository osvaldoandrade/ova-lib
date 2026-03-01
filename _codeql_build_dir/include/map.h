#ifndef HASH_H
#define HASH_H
#include "types.h"
#include "heap.h"
#include <pthread.h>

#define HASH_FUNC_COUNT 5
#define INITIAL_CAPACITY 20
#define LOAD_FACTOR 0.75

typedef struct map_entry {
    void *key;                  // Pointer to the key.
    void *data;                 // Pointer to the data associated with the key.
    struct map_entry *next;     // Pointer to the next node in case of a collision (chaining).
} map_entry;

/**
 * @file map.h
 * @brief Provides a hashmap data structure implementation.
 *
 * The map structure provides a hashmap data structure implementation with support for key-value pairs.
 * It allows for insertion, retrieval, and removal operations.
 * The map consider a set of function to determine the bucket in which a key-value pair should be stored.
 * This feature also supports thread-safe by conditional mutex application.
 */
typedef struct map {
    map_entry **buckets;       // Pointer to an array of pointers to map nodes.
    int capacity;              // Maximum number of buckets in the map table.
    int size;                  // Current number of elements in the map table.
    int (*hash_func)(void *key, int capacity);  // Function pointer to the map function.
    comparator key_compare;    // Function pointer to the key comparison function.
    pthread_mutex_t *lock;     // Lock attribute to be used in the case of thread-safe configuration.

    /**
     * @brief Function pointer for inserting a key-value pair into the map.
     *
     * This function pointer represents the ability to insert a key-value pair into the map.
     * The function takes a pointer to a map structure, a pointer to the key data, and a pointer to the value data as parameters.
     *
     * @param self A pointer to the map structure.
     * @param key A pointer to the key data to be inserted.
     * @param data A pointer to the value data to be inserted.
     *
     * @return This function does not return anything.
     */
    void (*put)(struct map *self, void *key, void *data);

    /**
     * @brief Function pointer for retrieving a value from the map based on a given key.
     *
     * This function pointer represents the ability to retrieve a value from the map based on a given key.
     * The function takes a pointer to a map structure and a pointer to the key data as parameters.
     *
     * @param self A pointer to the map structure.
     * @param key A pointer to the key data to search for in the map.
     *
     * @return A void pointer to the value associated with the given key, or NULL if the key does not exist in the map.
     */
    void *(*get)(struct map *self, void *key);

    /**
     * @brief Function pointer for removing a key-value pair from the map based on a given key.
     *
     * This function pointer represents the ability to remove a key-value pair from the map based on a given key.
     * The function takes a pointer to a map structure and a pointer to the key data as parameters.
     *
     * @param self A pointer to the map structure.
     * @param key A pointer to the key data to be removed.
     *
     * @return A void pointer to the value associated with the removed key, or NULL if the key does not exist in the map.
     */
    void *(*remove)(struct map *self, void *key);

    /**
     * @brief Function pointer for freeing the memory used by the map.
     *
     * This function pointer represents the ability to free the memory used by the map.
     * The function takes a pointer to a map structure as a parameter.
     *
     * @param self A pointer to the map structure to be freed.
     *
     * @return This function does not return anything.
     */
    void (*free)(struct map *self);

} map;

typedef enum {
    HASH_TABLE,
    HASH_MAP
} map_type;

/**
 * @brief Applies the Bernstein map function to a given key.
 *
 * This function computes the map of a given key using the Bernstein map method.
 *
 * @param key Pointer to the key to be hashed.
 * @param capacity The capacity of the map table, used to ensure the map value is within bounds.
 * @return The map value of the key.
 */
int bernstein_hash(void *key, int capacity);

/**
 * @brief Applies the FNV-1a map function to a given key.
 *
 * This function computes the map of a given key using the Fowler–Noll–Vo (FNV-1a) method.
 *
 * @param key Pointer to the key to be hashed.
 * @param capacity The capacity of the map table, used to ensure the map value is within bounds.
 * @return The map value of the key, reduced to fit within the map table capacity.
 */
int fnv1a_hash(void *key, int capacity);

/**
 * @brief Applies an XOR-based map function to a given key.
 *
 * This function computes the map of a given key using an XOR-based method.
 *
 * @param key Pointer to the key to be hashed.
 * @param capacity The capacity of the map table, used to ensure the map value is within bounds.
 * @return The map value of the key, reduced to fit within the map table capacity.
 */
int xor_hash(void *key, int capacity);

/**
 * @brief Applies a rotational map function to a given key.
 *
 * This function computes the map of a given key using a rotational method.
 *
 * @param key Pointer to the key to be hashed.
 * @param capacity The capacity of the map table, used to ensure the map value is within bounds.
 * @return The map value of the key, reduced to fit within the map table capacity.
 */
int rotational_hash(void *key, int capacity);

/**
 * @brief Applies an additive map function to a given key.
 *
 * This function computes the map of a given key using an additive method.
 *
 * @param key Pointer to the key to be hashed.
 * @param capacity The capacity of the map table, used to ensure the map value is within bounds.
 * @return The map value of the key, reduced to fit within the map table capacity.
 */
int additive_hash(void *key, int capacity);

/**
 * @brief Creates a new map.
 *
 * This function creates a new map based on the specified type, capacity, map function, and key comparison function.
 *
 * @param type The type of the map to be created. It can be either HASH_MAP or HASH_TABLE (thread-safe implementation).
 * @param capacity The maximum number of buckets in the map.
 * @param hash_func The map function to be used to determine the bucket for a key-value pair.
 * @param compare The function used for comparing keys in the map.
 * @return A pointer to the created map.
 */
map *create_map(map_type type, int capacity, int (*hash_func)(void *, int), comparator compare);

#endif // HASH_H
