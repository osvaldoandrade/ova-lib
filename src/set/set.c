#include "../../include/set.h"
#include "set_internal.h"

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
    x ^= x >> 16;
    x *= (uintptr_t)0x7feb352dU;
    x ^= x >> 15;
    x *= (uintptr_t)0x846ca68bU;
    x ^= x >> 16;
    return (int)(x % (uintptr_t)capacity);
}

static bool set_add_method(set *self, void *element) {
    set_impl *state = set_impl_from_public(self);
    return (state && state->ops && state->ops->add) ? state->ops->add(state, element) : false;
}

static void set_add_bulk_method(set *self, void **elements, int count) {
    if (!self || !elements || count <= 0) {
        return;
    }
    for (int i = 0; i < count; i++) {
        self->add(self, elements[i]);
    }
}

static bool set_contains_method(const set *self, void *element) {
    set_impl *state = set_impl_from_public(self);
    return (state && state->ops && state->ops->contains) ? state->ops->contains(state, element) : false;
}

static bool set_remove_method(set *self, void *element) {
    set_impl *state = set_impl_from_public(self);
    return (state && state->ops && state->ops->remove) ? state->ops->remove(state, element) : false;
}

static int set_size_method(const set *self) {
    set_impl *state = set_impl_from_public(self);
    return (state && state->ops && state->ops->size) ? state->ops->size(state) : 0;
}

static list *set_to_list_method(const set *self) {
    set_impl *state = set_impl_from_public(self);
    return (state && state->ops && state->ops->to_list) ? state->ops->to_list(state) : NULL;
}

static int sets_are_compatible(const set_impl *lhs, const set_impl *rhs) {
    if (!lhs || !rhs || !lhs->ops || !rhs->ops) {
        return 0;
    }
    if (!lhs->cmp || !rhs->cmp || lhs->cmp != rhs->cmp) {
        return 0;
    }
    if (lhs->hash || rhs->hash) {
        if (!lhs->hash || !rhs->hash || lhs->hash != rhs->hash) {
            return 0;
        }
    }
    return lhs->type == rhs->type;
}

static set *set_union_with_method(const set *self, const set *other);
static set *set_intersection_with_method(const set *self, const set *other);
static set *set_difference_with_method(const set *self, const set *other);
static bool set_is_subset_of_method(const set *self, const set *other);
static void set_free_method(set *self);
static set *set_clone_shallow_method(const set *self);
static set *set_clone_deep_method(const set *self, element_copier copier);

static set *create_set_with_capacity(set_type type, comparator cmp, hash_func_t hash, int capacity_hint) {
    set *out = (set *)calloc(1, sizeof(set));
    if (!out) {
        return NULL;
    }

    set_impl *state = (set_impl *)calloc(1, sizeof(set_impl));
    if (!state) {
        free(out);
        return NULL;
    }

    state->type = type;

    if (type == SET_HASH) {
        if (!cmp && !hash) {
            cmp = set_default_ptr_compare;
            hash = set_default_ptr_hash;
        }
        if (!cmp || !hash) {
            free(state);
            free(out);
            return NULL;
        }

        state->cmp = cmp;
        state->hash = hash;
        state->ops = &hash_set_ops;
        state->backend_impl = hash_set_create_impl(capacity_hint, cmp, hash);
    } else if (type == SET_TREE) {
        if (!cmp) {
            cmp = set_default_ptr_compare;
        }

        state->cmp = cmp;
        state->hash = NULL;
        state->ops = &tree_set_ops;
        state->backend_impl = tree_set_create_impl(cmp);
    } else {
        free(state);
        free(out);
        return NULL;
    }

    if (!state->backend_impl) {
        free(state);
        free(out);
        return NULL;
    }

    out->impl = state;
    out->add = set_add_method;
    out->add_bulk = set_add_bulk_method;
    out->contains = set_contains_method;
    out->remove = set_remove_method;
    out->size = set_size_method;
    out->to_list = set_to_list_method;
    out->union_with = set_union_with_method;
    out->intersection_with = set_intersection_with_method;
    out->difference_with = set_difference_with_method;
    out->is_subset_of = set_is_subset_of_method;
    out->free = set_free_method;
    out->clone_shallow = set_clone_shallow_method;
    out->clone_deep = set_clone_deep_method;
    return out;
}

static set *set_union_with_method(const set *self, const set *other) {
    set_impl *lhs = set_impl_from_public(self);
    set_impl *rhs = set_impl_from_public(other);
    if (!sets_are_compatible(lhs, rhs)) {
        return NULL;
    }

    int cap = self->size(self) + other->size(other);
    set *out = create_set_with_capacity(lhs->type, lhs->cmp, lhs->hash, cap);
    if (!out) {
        return NULL;
    }

    list *la = self->to_list(self);
    if (!la) {
        out->free(out);
        return NULL;
    }
    for (int i = 0, n = la->size(la); i < n; i++) {
        out->add(out, la->get(la, i));
    }
    la->free(la);

    list *lb = other->to_list(other);
    if (!lb) {
        out->free(out);
        return NULL;
    }
    for (int i = 0, n = lb->size(lb); i < n; i++) {
        out->add(out, lb->get(lb, i));
    }
    lb->free(lb);

    return out;
}

static set *set_intersection_with_method(const set *self, const set *other) {
    set_impl *lhs = set_impl_from_public(self);
    set_impl *rhs = set_impl_from_public(other);
    if (!sets_are_compatible(lhs, rhs)) {
        return NULL;
    }

    const set *iter = self;
    const set *match = other;
    if (other->size(other) < self->size(self)) {
        iter = other;
        match = self;
    }

    set *out = create_set_with_capacity(lhs->type, lhs->cmp, lhs->hash, iter->size(iter));
    if (!out) {
        return NULL;
    }

    list *items = iter->to_list(iter);
    if (!items) {
        out->free(out);
        return NULL;
    }

    for (int i = 0, n = items->size(items); i < n; i++) {
        void *element = items->get(items, i);
        if (match->contains(match, element)) {
            out->add(out, element);
        }
    }

    items->free(items);
    return out;
}

static set *set_difference_with_method(const set *self, const set *other) {
    set_impl *lhs = set_impl_from_public(self);
    set_impl *rhs = set_impl_from_public(other);
    if (!sets_are_compatible(lhs, rhs)) {
        return NULL;
    }

    set *out = create_set_with_capacity(lhs->type, lhs->cmp, lhs->hash, self->size(self));
    if (!out) {
        return NULL;
    }

    list *items = self->to_list(self);
    if (!items) {
        out->free(out);
        return NULL;
    }

    for (int i = 0, n = items->size(items); i < n; i++) {
        void *element = items->get(items, i);
        if (!other->contains(other, element)) {
            out->add(out, element);
        }
    }

    items->free(items);
    return out;
}

static bool set_is_subset_of_method(const set *self, const set *other) {
    set_impl *lhs = set_impl_from_public(self);
    set_impl *rhs = set_impl_from_public(other);
    if (!sets_are_compatible(lhs, rhs)) {
        return false;
    }

    list *items = self->to_list(self);
    if (!items) {
        return false;
    }

    bool result = true;
    for (int i = 0, n = items->size(items); i < n; i++) {
        if (!other->contains(other, items->get(items, i))) {
            result = false;
            break;
        }
    }

    items->free(items);
    return result;
}

static void set_free_method(set *self) {
    if (!self) {
        return;
    }

    set_impl *state = set_impl_from_public(self);
    if (state) {
        if (state->ops && state->ops->destroy) {
            state->ops->destroy(state);
        }
        free(state);
        self->impl = NULL;
    }

    free(self);
}

set *create_set(set_type type, comparator cmp, hash_func_t hash) {
    return create_set_with_capacity(type, cmp, hash, 0);
}

static set *set_clone_shallow_method(const set *self) {
    if (!self) {
        return NULL;
    }

    set_impl *state = set_impl_from_public(self);
    if (!state) {
        return NULL;
    }

    list *items = self->to_list(self);
    if (!items) {
        return NULL;
    }

    int n = items->size(items);
    set *copy = create_set_with_capacity(state->type, state->cmp, state->hash, n);
    if (!copy) {
        items->free(items);
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        copy->add(copy, items->get(items, i));
    }

    items->free(items);
    copy->user_data = self->user_data;
    return copy;
}

static set *set_clone_deep_method(const set *self, element_copier copier) {
    if (!self || !copier) {
        return NULL;
    }

    set_impl *state = set_impl_from_public(self);
    if (!state) {
        return NULL;
    }

    list *items = self->to_list(self);
    if (!items) {
        return NULL;
    }

    int n = items->size(items);
    set *copy = create_set_with_capacity(state->type, state->cmp, state->hash, n);
    if (!copy) {
        items->free(items);
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        void *dup = copier(items->get(items, i));
        if (!dup) {
            items->free(items);
            copy->free(copy);
            return NULL;
        }
        copy->add(copy, dup);
    }

    items->free(items);
    copy->user_data = self->user_data;
    return copy;
}
