#include "../../include/map.h"
#include "hash_map.h"

map *create_map(map_type type, int capacity, int (*hash_func)(void *, int), comparator compare) {
    switch (type) {
        case HASH_MAP:
            return create_hash_map(capacity, hash_func, compare, 0);
        case HASH_TABLE:
            return create_hash_map(capacity, hash_func, compare, 1);
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

int bernstein_hash(void *key, int capacity) {
    if (key == NULL || capacity <= 0) {
        return 0;
    }

    unsigned char *str = (unsigned char *)key;
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + (unsigned long)c;
    }
    return (int)(hash % (unsigned long)capacity);
}

int fnv1a_hash(void *key, int capacity) {
    if (key == NULL || capacity <= 0) {
        return 0;
    }

    unsigned char *str = (unsigned char *)key;
    unsigned long hash = 2166136261u;
    while (*str) {
        hash ^= (unsigned long)(*str++);
        hash *= 16777619;
    }
    return (int)(hash % (unsigned long)capacity);
}

int xor_hash(void *key, int capacity) {
    if (key == NULL || capacity <= 0) {
        return 0;
    }

    unsigned char *str = (unsigned char *)key;
    unsigned long hash = 0;
    while (*str) {
        hash ^= (hash << 5) + (hash >> 2) + (unsigned long)(*str++);
    }
    return (int)(hash % (unsigned long)capacity);
}

int rotational_hash(void *key, int capacity) {
    if (key == NULL || capacity <= 0) {
        return 0;
    }

    unsigned char *str = (unsigned char *)key;
    unsigned long hash = 0;
    while (*str) {
        hash = (hash << 4) ^ (hash >> 28) ^ (unsigned long)(*str++);
    }
    return (int)(hash % (unsigned long)capacity);
}

int additive_hash(void *key, int capacity) {
    if (key == NULL || capacity <= 0) {
        return 0;
    }

    unsigned char *str = (unsigned char *)key;
    unsigned long hash = 0;
    while (*str) {
        hash += (unsigned long)(*str++);
    }
    return (int)(hash % (unsigned long)capacity);
}
