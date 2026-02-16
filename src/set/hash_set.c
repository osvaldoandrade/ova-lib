#include "set_internal.h"

#include "../../include/map.h"

#include <stdlib.h>

typedef struct {
    map *m;
} hash_set_impl;

static char hash_set_marker;

static hash_set_impl *hash_set_impl_from_set(const set *s) {
    return s ? (hash_set_impl *)s->impl : NULL;
}

static bool hash_set_add(set *s, void *element) {
    if (!s || !element) {
        return false;
    }

    hash_set_impl *impl = hash_set_impl_from_set(s);
    if (!impl || !impl->m) {
        return false;
    }

    if (impl->m->get(impl->m, element) != NULL) {
        return false;
    }

    int before = impl->m->size;
    impl->m->put(impl->m, element, &hash_set_marker);
    return impl->m->size > before;
}

static bool hash_set_contains(const set *s, void *element) {
    if (!s || !element) {
        return false;
    }

    hash_set_impl *impl = hash_set_impl_from_set(s);
    if (!impl || !impl->m) {
        return false;
    }

    return impl->m->get(impl->m, element) != NULL;
}

static bool hash_set_remove(set *s, void *element) {
    if (!s || !element) {
        return false;
    }

    hash_set_impl *impl = hash_set_impl_from_set(s);
    if (!impl || !impl->m) {
        return false;
    }

    return impl->m->remove(impl->m, element) != NULL;
}

static int hash_set_size(const set *s) {
    hash_set_impl *impl = hash_set_impl_from_set(s);
    if (!impl || !impl->m) {
        return 0;
    }
    return impl->m->size;
}

static list *hash_set_to_list(const set *s) {
    hash_set_impl *impl = hash_set_impl_from_set(s);
    if (!impl || !impl->m) {
        return NULL;
    }

    int n = impl->m->size;
    list *out = create_list(ARRAY_LIST, n > 0 ? n : 4, NULL);
    if (!out) {
        return NULL;
    }

    for (int i = 0; i < impl->m->capacity; i++) {
        map_entry *node = impl->m->buckets ? impl->m->buckets[i] : NULL;
        while (node) {
            out->insert(out, node->key, out->size(out));
            node = node->next;
        }
    }

    return out;
}

static void hash_set_destroy(set *s) {
    if (!s) {
        return;
    }

    hash_set_impl *impl = hash_set_impl_from_set(s);
    if (impl) {
        if (impl->m) {
            impl->m->free(impl->m);
            impl->m = NULL;
        }
        free(impl);
        s->impl = NULL;
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

