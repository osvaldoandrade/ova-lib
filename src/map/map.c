/**
 * @file map.c
 * @brief Factory functions for map instances.
 */

#include "../../include/map.h"
#include "hash_map.h"

/**
 * @brief Factory function for creating different map implementations.
 *
 * @param type Desired map type (HASH_MAP or HASH_TABLE).
 * @param capacity Initial capacity of the structure.
 * @param hash_func Optional hash function.
 * @param compare Comparator for keys.
 * @return Pointer to a new map or NULL on failure.
 */
map *create_map(map_type type, int capacity, int (*hash_func)(void *, int), comparator compare) {
    switch (type) {
        case HASH_MAP:
            return create_hash_map(capacity,hash_func, compare, 0);
        case HASH_TABLE:
            return create_hash_map(capacity,hash_func, compare, 1);
        default:
            return NULL;
    }
}

hash_func_t hash_functions[HASH_FUNC_COUNT] = {
        bernstein_hash,
        fnv1a_hash,
        xor_hash,
        rotational_hash,
        additive_hash
};

/**
 * @brief Applies the Bernstein map function to a given key.
 *
 * This function computes the map of a given key using the Bernstein map method.
 *
 * @param key Pointer to the key to be hashed.
 * @param capacity The capacity of the map table, used to ensure the map value is within bounds.
 * @return The map value of the key.
 */
int bernstein_hash(void *key, int capacity) {
    if (key == NULL) {
        return 0;
    }
    unsigned char *str = (unsigned char *) key;
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // map * 33 + c
    }
    return (int) (hash % capacity);
}

/**
 * @brief Applies the FNV-1a map function to a given key.
 *
 * This function computes the map of a given key using the Fowler–Noll–Vo (FNV-1a) method.
 *
 * @param key Pointer to the key to be hashed.
 * @param capacity The capacity of the map table, used to ensure the map value is within bounds.
 * @return The map value of the key, reduced to fit within the map table capacity.
 */
int fnv1a_hash(void *key, int capacity) {
    if (key == NULL) {
        return 0; // Use 0 ou outro valor fixo para map de chave NULL
    }
    unsigned char *str = (unsigned char *) key;
    unsigned long hash = 2166136261u;
    while (*str) {
        hash ^= (unsigned long) (*str++);
        hash *= 16777619;
    }
    return (int) (hash % capacity);
}

/**
 * @brief Applies an XOR-based map function to a given key.
 *
 * This function computes the map of a given key using an XOR-based method.
 *
 * @param key Pointer to the key to be hashed.
 * @param capacity The capacity of the map table, used to ensure the map value is within bounds.
 * @return The map value of the key, reduced to fit within the map table capacity.
 */
int xor_hash(void *key, int capacity) {
    if (key == NULL) {
        return 0; // Use 0 ou outro valor fixo para map de chave NULL
    }
    unsigned char *str = (unsigned char *) key;
    unsigned long hash = 0;
    while (*str) {
        hash ^= (hash << 5) + (hash >> 2) + (unsigned long) (*str++);
    }
    return (int) (hash % capacity);
}

/**
 * @brief Applies a rotational map function to a given key.
 *
 * This function computes the map of a given key using a rotational method.
 *
 * @param key Pointer to the key to be hashed.
 * @param capacity The capacity of the map table, used to ensure the map value is within bounds.
 * @return The map value of the key, reduced to fit within the map table capacity.
 */
int rotational_hash(void *key, int capacity) {
    if (key == NULL) {
        return 0;
    }
    unsigned char *str = (unsigned char *) key;
    unsigned long hash = 0;
    while (*str) {
        hash = (hash << 4) ^ (hash >> 28) ^ (unsigned long) (*str++);
    }
    return (int) (hash % capacity);
}

/**
 * @brief Applies an additive map function to a given key.
 *
 * This function computes the map of a given key using an additive method.
 *
 * @param key Pointer to the key to be hashed.
 * @param capacity The capacity of the map table, used to ensure the map value is within bounds.
 * @return The map value of the key, reduced to fit within the map table capacity.
 */
int additive_hash(void *key, int capacity) {
    if (key == NULL) {
        return 0;
    }
    unsigned char *str = (unsigned char *) key;
    unsigned long hash = 0;
    while (*str) {
        hash += (unsigned long) (*str++);
    }
    return (int) (hash % capacity);
}