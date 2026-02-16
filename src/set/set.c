#include "../../include/set.h"
#include "set_internal.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

int set_default_ptr_compare(const void *a, const void *b) {
    uintptr_t lhs = (uintptr_t)a;
    uintptr_t rhs = (uintptr_t)b;
    return (lhs > rhs) - (lhs < rhs);
}

int set_default_ptr_hash(void *key, int capacity) {
    if (!key || capacity <= 0) {
        return 0;
    }

    uintptr_t x = (uintptr_t)key;
    /* Mix pointer bits; constants chosen for good avalanche (32-bit friendly). */
    x ^= x >> 16;
    x *= (uintptr_t)0x7feb352dU;
    x ^= x >> 15;
    x *= (uintptr_t)0x846ca68bU;
    x ^= x >> 16;

    return (int)(x % (uintptr_t)capacity);
}

static set *create_set_with_capacity(set_type type, comparator cmp, hash_func_t hash, int capacity_hint) {
    set *s = (set *)calloc(1, sizeof(set));
    if (!s) {
        return NULL;
    }

    s->type = type;

    if (type == SET_HASH) {
        if (!cmp && !hash) {
            cmp = set_default_ptr_compare;
            hash = set_default_ptr_hash;
        }
        if (!cmp || !hash) {
            free(s);
            return NULL;
        }

        s->cmp = cmp;
        s->hash = hash;
        s->ops = &hash_set_ops;
        s->impl = hash_set_create_impl(capacity_hint, cmp, hash);
        if (!s->impl) {
            free(s);
            return NULL;
        }
        return s;
    }

    if (type == SET_TREE) {
        if (!cmp) {
            cmp = set_default_ptr_compare;
        }
        s->cmp = cmp;
        s->hash = NULL;
        s->ops = &tree_set_ops;
        s->impl = tree_set_create_impl(cmp);
        if (!s->impl) {
            free(s);
            return NULL;
        }
        return s;
    }

    free(s);
    return NULL;
}

set *create_set(set_type type, comparator cmp, hash_func_t hash) {
    return create_set_with_capacity(type, cmp, hash, 0);
}

void set_free(set *s) {
    if (!s) {
        return;
    }
    if (s->ops && s->ops->destroy) {
        s->ops->destroy(s);
    }
    free(s);
}

bool set_add(set *s, void *element) {
    if (!s || !s->ops || !s->ops->add || !element) {
        return false;
    }
    return s->ops->add(s, element);
}

bool set_contains(const set *s, void *element) {
    if (!s || !s->ops || !s->ops->contains || !element) {
        return false;
    }
    return s->ops->contains(s, element);
}

bool set_remove(set *s, void *element) {
    if (!s || !s->ops || !s->ops->remove || !element) {
        return false;
    }
    return s->ops->remove(s, element);
}

int set_size(const set *s) {
    if (!s || !s->ops || !s->ops->size) {
        return 0;
    }
    return s->ops->size(s);
}

list *set_to_list(const set *s) {
    if (!s || !s->ops || !s->ops->to_list) {
        return NULL;
    }
    return s->ops->to_list(s);
}

static int sets_are_compatible(const set *a, const set *b) {
    /* Both sets must be non-NULL and have valid ops tables. */
    if (!a || !b || !a->ops || !b->ops) {
        return 0;
    }

    /* Require a valid comparator on both sets, and that they are the same. */
    if (!a->cmp || !b->cmp || a->cmp != b->cmp) {
        return 0;
    }

    /* If either set uses hashing, require both to have the same hash function. */
    if (a->hash || b->hash) {
        if (!a->hash || !b->hash || a->hash != b->hash) {
            return 0;
        }
    }

    /* Ensure the underlying set implementation/type is the same. */
    if (a->type != b->type) {
        return 0;
    }
    /* Set algebra assumes compatible equality semantics across sets. */
    return 1;
}

set *set_union(const set *a, const set *b) {
    if (!sets_are_compatible(a, b)) {
        return NULL;
    }

    /* Compute capacity hint in size_t to avoid signed overflow. */
    size_t size_a = (size_t)set_size(a);
    size_t size_b = (size_t)set_size(b);
    size_t cap_sz = size_a + size_b;
    /* Clamp to INT_MAX if sum exceeds it. */
    int cap = (cap_sz > (size_t)INT_MAX) ? INT_MAX : (int)cap_sz;
    set *out = create_set_with_capacity(a->type, a->cmp, a->hash, cap);
    if (!out) {
        return NULL;
    }

    list *la = set_to_list(a);
    if (!la) {
        set_free(out);
        return NULL;
    }
    for (int i = 0, n = la->size(la); i < n; i++) {
        set_add(out, la->get(la, i));
    }
    la->free(la);

    list *lb = set_to_list(b);
    if (!lb) {
        set_free(out);
        return NULL;
    }
    for (int i = 0, n = lb->size(lb); i < n; i++) {
        set_add(out, lb->get(lb, i));
    }
    lb->free(lb);

    return out;
}

set *set_intersection(const set *a, const set *b) {
    if (!sets_are_compatible(a, b)) {
        return NULL;
    }

    const set *iter = a;
    const set *other = b;
    if (set_size(b) < set_size(a)) {
        iter = b;
        other = a;
    }

    int cap = set_size(iter);
    set *out = create_set_with_capacity(a->type, a->cmp, a->hash, cap);
    if (!out) {
        return NULL;
    }

    list *li = set_to_list(iter);
    if (!li) {
        set_free(out);
        return NULL;
    }

    for (int i = 0, n = li->size(li); i < n; i++) {
        void *e = li->get(li, i);
        if (set_contains(other, e)) {
            set_add(out, e);
        }
    }

    li->free(li);
    return out;
}

set *set_difference(const set *a, const set *b) {
    if (!sets_are_compatible(a, b)) {
        return NULL;
    }

    int cap = set_size(a);
    set *out = create_set_with_capacity(a->type, a->cmp, a->hash, cap);
    if (!out) {
        return NULL;
    }

    list *la = set_to_list(a);
    if (!la) {
        set_free(out);
        return NULL;
    }

    for (int i = 0, n = la->size(la); i < n; i++) {
        void *e = la->get(la, i);
        if (!set_contains(b, e)) {
            set_add(out, e);
        }
    }

    la->free(la);
    return out;
}

bool set_is_subset(const set *a, const set *b) {
    if (!sets_are_compatible(a, b)) {
        return false;
    }

    list *la = set_to_list(a);
    if (!la) {
        return false;
    }

    bool ok = true;
    for (int i = 0, n = la->size(la); i < n; i++) {
        if (!set_contains(b, la->get(la, i))) {
            ok = false;
            break;
        }
    }

    la->free(la);
    return ok;
}

