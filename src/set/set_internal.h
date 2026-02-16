#ifndef SET_INTERNAL_H
#define SET_INTERNAL_H

#include "../../include/set.h"

typedef struct set_ops {
    bool (*add)(set *s, void *element);
    bool (*contains)(const set *s, void *element);
    bool (*remove)(set *s, void *element);
    int (*size)(const set *s);
    list *(*to_list)(const set *s);
    void (*destroy)(set *s);
} set_ops;

struct set {
    set_type type;
    comparator cmp;
    hash_func_t hash;
    void *impl;
    const set_ops *ops;
};

int set_default_ptr_compare(const void *a, const void *b);
int set_default_ptr_hash(void *key, int capacity);

void *hash_set_create_impl(int capacity, comparator cmp, hash_func_t hash);
extern const set_ops hash_set_ops;

void *tree_set_create_impl(comparator cmp);
extern const set_ops tree_set_ops;

#endif // SET_INTERNAL_H

