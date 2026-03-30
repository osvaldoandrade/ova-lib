#ifndef SET_INTERNAL_H
#define SET_INTERNAL_H

#include "../../include/set.h"

typedef struct set_impl set_impl;

typedef struct set_ops {
    bool (*add)(set_impl *state, void *element);
    bool (*contains)(const set_impl *state, void *element);
    bool (*remove)(set_impl *state, void *element);
    int (*size)(const set_impl *state);
    list *(*to_list)(const set_impl *state);
    void (*destroy)(set_impl *state);
} set_ops;

struct set_impl {
    set_type type;
    comparator cmp;
    hash_func_t hash;
    void *backend_impl;
    const set_ops *ops;
};

static inline set_impl *set_impl_from_public(const set *s) {
    return s ? (set_impl *)s->impl : NULL;
}

int set_default_ptr_compare(const void *a, const void *b);
int set_default_ptr_hash(void *key, int capacity);

void *hash_set_create_impl(int capacity, comparator cmp, hash_func_t hash);
extern const set_ops hash_set_ops;

void *tree_set_create_impl(comparator cmp);
extern const set_ops tree_set_ops;

#endif // SET_INTERNAL_H
