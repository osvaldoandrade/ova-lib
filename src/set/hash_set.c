#include "set_internal.h"

#include "../map/map_internal.h"

#include <stdlib.h>

typedef struct {
    map *m;
} hash_set_impl;

static char hash_set_marker;

static hash_set_impl *hash_set_impl_from_state(const set_impl *state) {
    return state ? (hash_set_impl *)state->backend_impl : NULL;
}

static bool hash_set_add(set_impl *state, void *element) {
    if (!state || !element) {
        return false;
    }

    hash_set_impl *impl = hash_set_impl_from_state(state);
    if (!impl || !impl->m) {
        return false;
    }

    if (impl->m->get(impl->m, element) != NULL) {
        return false;
    }

    int before = impl->m->size(impl->m);
    impl->m->put(impl->m, element, &hash_set_marker);
    return impl->m->size(impl->m) > before;
}

static bool hash_set_contains(const set_impl *state, void *element) {
    if (!state || !element) {
        return false;
    }

    hash_set_impl *impl = hash_set_impl_from_state(state);
    if (!impl || !impl->m) {
        return false;
    }

    return impl->m->get(impl->m, element) != NULL;
}

static bool hash_set_remove(set_impl *state, void *element) {
    if (!state || !element) {
        return false;
    }

    hash_set_impl *impl = hash_set_impl_from_state(state);
    if (!impl || !impl->m) {
        return false;
    }

    return impl->m->remove(impl->m, element) != NULL;
}

static int hash_set_size(const set_impl *state) {
    hash_set_impl *impl = hash_set_impl_from_state(state);
    return (impl && impl->m) ? impl->m->size(impl->m) : 0;
}

static list *hash_set_to_list(const set_impl *state) {
    hash_set_impl *impl = hash_set_impl_from_state(state);
    if (!impl || !impl->m) {
        return NULL;
    }

    map_impl *map_state = map_impl_from_map(impl->m);
    if (!map_state) {
        return NULL;
    }

    int n = impl->m->size(impl->m);
    list *out = create_list(ARRAY_LIST, n > 0 ? n : 4, NULL);
    if (!out) {
        return NULL;
    }

    for (int i = 0; i < map_state->capacity; i++) {
        map_entry *node = map_state->buckets ? map_state->buckets[i] : NULL;
        while (node) {
            out->insert(out, node->key, out->size(out));
            node = node->next;
        }
    }

    return out;
}

static void hash_set_destroy(set_impl *state) {
    if (!state) {
        return;
    }

    hash_set_impl *impl = hash_set_impl_from_state(state);
    if (impl) {
        if (impl->m) {
            impl->m->free(impl->m);
            impl->m = NULL;
        }
        free(impl);
        state->backend_impl = NULL;
    }
}

void *hash_set_create_impl(int capacity, comparator cmp, hash_func_t hash) {
    if (!cmp || !hash) {
        return NULL;
    }

    map *m = create_map(HASH_MAP, capacity, hash, cmp);
    if (!m) {
        return NULL;
    }

    hash_set_impl *impl = (hash_set_impl *)calloc(1, sizeof(hash_set_impl));
    if (!impl) {
        m->free(m);
        return NULL;
    }

    impl->m = m;
    return impl;
}

const set_ops hash_set_ops = {
    .add = hash_set_add,
    .contains = hash_set_contains,
    .remove = hash_set_remove,
    .size = hash_set_size,
    .to_list = hash_set_to_list,
    .destroy = hash_set_destroy,
};
