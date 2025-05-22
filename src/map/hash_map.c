/**
 * @file hash_map.c
 * @brief Chained hash map implementation.
 */

#include "hash_map.h"
#include <stdlib.h>
#include <string.h>

void hash_insert(map *self, void *key, void *data);
void *hash_get(map *self, void *key);
void *hash_remove(map *self, void *key);
void hash_free(map *self);

/**
 * @brief Create a hash map using separate chaining.
 *
 * @param capacity Initial table capacity.
 * @param hash_func Hash function to distribute keys.
 * @param key_compare Comparator used to compare keys.
 * @param thread_safe Non-zero to allocate a mutex for thread safety.
 * @return Pointer to the created map or NULL on failure.
 */
map *create_hash_map(int capacity, int (*hash_func)(void *, int), comparator key_compare, int thread_safe) {
    if (capacity < INITIAL_CAPACITY) {
        capacity = INITIAL_CAPACITY;
    }
    map *table = (map *) malloc(sizeof(map));
    if (table) {
        table->buckets = (map_entry **) calloc(capacity, sizeof(map_entry *));
        if (!table->buckets) {
            free(table);
            return NULL;
        }
        table->capacity = capacity;
        table->size = 0;
        table->hash_func = hash_func ? hash_func : bernstein_hash;
        table->key_compare = key_compare;
        table->put = hash_insert;
        table->get = hash_get;
        table->remove = hash_remove;
        table->free = hash_free;

        if (thread_safe) {
            table->lock = malloc(sizeof(pthread_mutex_t));
            if (table->lock) {
                pthread_mutex_init(table->lock, NULL);
            } else {
                free(table->buckets);
                free(table);
                return NULL;
            }
        } else {
            table->lock = NULL;
        }
    }
    return table;
}

/**
 * @brief Resizes and rehashes the map table.
 *
 * This function resizes the map table by doubling its capacity and rehashes
 * all the entries. It allocates new memory for the expanded buckets and moves
 * the entries to the new buckets according to their new index.
 *
 * @param ht Pointer to the map table.
 */
void resize_and_rehash(map *ht) {
    int old_capacity = ht->capacity;
    ht->capacity *= 2;
    map_entry **new_buckets = calloc(ht->capacity, sizeof(map_entry*));

    if (!new_buckets) {
        return;
    }

    for (int i = 0; i < old_capacity; i++) {
        map_entry *node = ht->buckets[i];
        while (node) {
            map_entry *next_node = node->next;
            int new_index = ht->hash_func(node->key, ht->capacity);

            node->next = new_buckets[new_index];
            new_buckets[new_index] = node;
            node = next_node;
        }
    }

    free(ht->buckets);
    ht->buckets = new_buckets;
}

/**
 * @brief Inserts a key-value pair into the map table.
 *
 * This function inserts the given key-value pair into the map table
 * at the appropriate position based on the calculated index using the map function.
 * If the load factor of the map table exceeds the defined load factor, it is resized
 * and rehashed before insertion.
 *
 * @param self Pointer to the map table.
 * @param key Pointer to the key.
 * @param data Pointer to the data associated with the key.
 */
void hash_insert(map *self, void *key, void *data) {
    if (self->lock) pthread_mutex_lock(self->lock);

    float load_factor = (float)self->size / self->capacity;
    if (load_factor > LOAD_FACTOR) {
        resize_and_rehash(self);
    }

    int index = self->hash_func(key, self->capacity);
    map_entry *node = self->buckets[index];

    while (node) {
        if (self->key_compare(node->key, key) == 0) {
            node->data = data;  // Atualizar os dados se a chave já existir
            if (self->lock) pthread_mutex_unlock(self->lock);
            return;
        }
        node = node->next;
    }

    map_entry *new_node = malloc(sizeof(map_entry));
    if (new_node) {
        new_node->key = key;
        new_node->data = data;
        new_node->next = self->buckets[index];
        self->buckets[index] = new_node;
        self->size++;
    }

    if (self->lock) pthread_mutex_unlock(self->lock);
}

/**
 * @brief Retrieves the value associated with a given key in the map table.
 *
 * This function returns the value associated with the specified key if it exists in the map table.
 * If the key is found in the map table, the associated value is returned. Otherwise, NULL is returned.
 *
 * @param self A pointer to the map table.
 * @param key A pointer to the key to search for.
 * @return The value associated with the specified key, or NULL if the key is not found.
 */
void *hash_get(map *self, void *key) {
    if (self->lock) pthread_mutex_lock(self->lock);

    int index = (key == NULL) ? 0 : self->hash_func(key, self->capacity);
    map_entry *node = self->buckets[index];
    while (node) {
        if ((node->key == key) || (key != NULL && self->key_compare(node->key, key) == 0)) {
            void *result = node->data;
            if (self->lock) pthread_mutex_unlock(self->lock);
            return result;
        }
        node = node->next;
    }

    if (self->lock) pthread_mutex_unlock(self->lock);
    return NULL;
}

/**
 * Removes an entry from the map table based on the specified key.
 *
 * @param self The map table to remove the entry from.
 * @param key The key of the entry to be removed.
 * @return The data associated with the removed entry, or NULL if the entry was not found.
 */
void *hash_remove(map *self, void *key) {
    if (self->lock) pthread_mutex_lock(self->lock);

    int index = self->hash_func(key, self->capacity);
    map_entry *prev = NULL;
    map_entry *current = self->buckets[index];

    while (current) {
        if (self->key_compare(current->key, key) == 0) {
            void *data = current->data;
            if (prev == NULL) {
                self->buckets[index] = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            self->size--;

            if (self->lock) pthread_mutex_unlock(self->lock);
            return data;
        }

        prev = current;
        current = current->next;
    }

    if (self->lock) pthread_mutex_unlock(self->lock);
    return NULL;
}

/**
 * @brief Frees the memory allocated for the map table.
 *
 * This function frees the memory allocated for the map table and its elements.
 *
 * @param self Pointer to the map table.
 */
void hash_free(map *self) {
    if (self->lock) {
        pthread_mutex_lock(self->lock);
    }

    for (int i = 0; i < self->capacity; i++) {
        map_entry *node = self->buckets[i];
        while (node) {
            map_entry *tmp = node;
            node = node->next;
            free(tmp);
        }
    }
    free(self->buckets);

    if (self->lock) {
        pthread_mutex_unlock(self->lock);
        pthread_mutex_destroy(self->lock);
        free(self->lock);
    }

    free(self);
}
