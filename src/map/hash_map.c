#include "hash_map.h"
#include "../utils/capacity_utils.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static bool safe_key_equals(void *k1, void *k2, comparator cmp) {
    if (k1 == NULL && k2 == NULL) return true;
    if (k1 == NULL || k2 == NULL) return false;
    return cmp(k1, k2) == 0;
}

static int map_keys_equal(map_impl *impl, void *lhs, void *rhs) {
    if (!impl || !impl->key_compare) {
        return (lhs == rhs) ? 1 : 0;
    }
    return safe_key_equals(lhs, rhs, impl->key_compare) ? 1 : 0;
}

static int map_bucket_index(map_impl *impl, void *key) {
    if (!impl || impl->capacity <= 0) {
        return 0;
    }
    if (key == NULL) {
        return 0;
    }
    return impl->hash_func(key, impl->capacity);
}

static void resize_and_rehash(map_impl *impl) {
    int old_capacity = impl->capacity;
    int new_capacity = safe_double_capacity(old_capacity);
    if (new_capacity == old_capacity) {
        return;
    }

    map_entry **new_buckets = calloc((size_t)new_capacity, sizeof(map_entry *));
    if (!new_buckets) {
        return;
    }

    for (int i = 0; i < old_capacity; i++) {
        map_entry *node = impl->buckets[i];
        while (node) {
            map_entry *next_node = node->next;
            int new_index = node->key ? impl->hash_func(node->key, new_capacity) : 0;
            node->next = new_buckets[new_index];
            new_buckets[new_index] = node;
            node = next_node;
        }
    }

    free(impl->buckets);
    impl->buckets = new_buckets;
    impl->capacity = new_capacity;
}

static ova_error_code hash_insert(map *self, void *key, void *data) {
    map_impl *impl = map_impl_from_map(self);
    if (!impl) {
        return OVA_ERROR_INVALID_ARG;
    }

    if (impl->lock) {
        pthread_mutex_lock(impl->lock);
    }

    double load_factor = (double)impl->size / (double)impl->capacity;
    if (load_factor > LOAD_FACTOR) {
        resize_and_rehash(impl);
    }

    int index = map_bucket_index(impl, key);
    map_entry *node = impl->buckets[index];
    while (node) {
        if (map_keys_equal(impl, node->key, key)) {
            node->data = data;
            if (impl->lock) {
                pthread_mutex_unlock(impl->lock);
            }
            return OVA_SUCCESS;
        }
        node = node->next;
    }

    map_entry *new_node = (map_entry *)malloc(sizeof(map_entry));
    if (!new_node) {
        if (impl->lock) {
            pthread_mutex_unlock(impl->lock);
        }
        return OVA_ERROR_MEMORY;
    }
    new_node->key = key;
    new_node->data = data;
    new_node->next = impl->buckets[index];
    impl->buckets[index] = new_node;
    impl->size++;

    if (impl->lock) {
        pthread_mutex_unlock(impl->lock);
    }
    return OVA_SUCCESS;
}

static ova_error_code hash_put_bulk(map *self, void **keys, void **values, int count) {
    if (!self || !keys || !values || count <= 0) {
        return OVA_ERROR_INVALID_ARG;
    }

    for (int i = 0; i < count; i++) {
        ova_error_code err = self->put(self, keys[i], values[i]);
        if (err != OVA_SUCCESS) {
            return err;
        }
    }
    return OVA_SUCCESS;
}

static void *hash_get(map *self, void *key) {
    map_impl *impl = map_impl_from_map(self);
    if (!impl) {
        return NULL;
    }

    if (impl->lock) {
        pthread_mutex_lock(impl->lock);
    }

    int index = map_bucket_index(impl, key);
    map_entry *node = impl->buckets[index];
    while (node) {
        if (map_keys_equal(impl, node->key, key)) {
            void *result = node->data;
            if (impl->lock) {
                pthread_mutex_unlock(impl->lock);
            }
            return result;
        }
        node = node->next;
    }

    if (impl->lock) {
        pthread_mutex_unlock(impl->lock);
    }
    return NULL;
}

static void *hash_remove(map *self, void *key) {
    map_impl *impl = map_impl_from_map(self);
    if (!impl) {
        return NULL;
    }

    if (impl->lock) {
        pthread_mutex_lock(impl->lock);
    }

    int index = map_bucket_index(impl, key);
    map_entry *prev = NULL;
    map_entry *current = impl->buckets[index];
    while (current) {
        if (map_keys_equal(impl, current->key, key)) {
            void *data = current->data;
            if (!prev) {
                impl->buckets[index] = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            impl->size--;

            if (impl->lock) {
                pthread_mutex_unlock(impl->lock);
            }
            return data;
        }
        prev = current;
        current = current->next;
    }

    if (impl->lock) {
        pthread_mutex_unlock(impl->lock);
    }
    return NULL;
}

static int hash_size(const map *self) {
    map_impl *impl = map_impl_from_map(self);
    return impl ? impl->size : 0;
}

static int hash_capacity(const map *self) {
    map_impl *impl = map_impl_from_map(self);
    return impl ? impl->capacity : 0;
}

static void hash_free(map *self) {
    if (!self) {
        return;
    }

    map_impl *impl = map_impl_from_map(self);
    if (impl) {
        if (impl->lock) {
            pthread_mutex_lock(impl->lock);
        }

        for (int i = 0; i < impl->capacity; i++) {
            map_entry *node = impl->buckets[i];
            while (node) {
                map_entry *tmp = node;
                node = node->next;
                free(tmp);
            }
        }
        free(impl->buckets);

        if (impl->lock) {
            pthread_mutex_unlock(impl->lock);
            pthread_mutex_destroy(impl->lock);
            free(impl->lock);
        }

        free(impl);
        self->impl = NULL;
    }

    free(self);
}

map *create_hash_map(int capacity, int (*hash_func)(void *, int), comparator key_compare, int thread_safe) {
    if (capacity < INITIAL_CAPACITY) {
        capacity = INITIAL_CAPACITY;
    }

    map *out = (map *)calloc(1, sizeof(map));
    if (!out) {
        return NULL;
    }

    map_impl *impl = (map_impl *)calloc(1, sizeof(map_impl));
    if (!impl) {
        free(out);
        return NULL;
    }

    impl->buckets = (map_entry **)calloc((size_t)capacity, sizeof(map_entry *));
    if (!impl->buckets) {
        free(impl);
        free(out);
        return NULL;
    }

    impl->capacity = capacity;
    impl->size = 0;
    impl->hash_func = hash_func ? hash_func : bernstein_hash;
    impl->key_compare = key_compare;
    impl->lock = NULL;

    if (thread_safe) {
        impl->lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
        if (!impl->lock) {
            free(impl->buckets);
            free(impl);
            free(out);
            return NULL;
        }
        if (pthread_mutex_init(impl->lock, NULL) != 0) {
            free(impl->lock);
            free(impl->buckets);
            free(impl);
            free(out);
            return NULL;
        }
    }

    out->impl = impl;
    out->put = hash_insert;
    out->put_bulk = hash_put_bulk;
    out->get = hash_get;
    out->remove = hash_remove;
    out->size = hash_size;
    out->capacity = hash_capacity;
    out->free = hash_free;

    return out;
}
