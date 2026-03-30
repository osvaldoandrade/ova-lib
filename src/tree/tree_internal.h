#ifndef TREE_INTERNAL_H
#define TREE_INTERNAL_H

#include "../../include/tree.h"

typedef enum {
    RB_RED = 0,
    RB_BLACK = 1
} rb_color;

typedef struct tree_node tree_node;
typedef struct tree_impl tree_impl;

struct tree_node {
    void *key;
    void *value;
    tree_node *left;
    tree_node *right;
    tree_node *parent;
    int height;
    rb_color color;
};

struct tree_impl {
    tree_type type;
    comparator cmp;
    tree_node *root;
    tree_node *nil;
    size_t size;
};

static inline tree_impl *tree_impl_from_tree(const tree *t) {
    return t ? (tree_impl *)t->impl : NULL;
}

static inline int tree_node_is_nil(const tree_impl *t, const tree_node *n) {
    if (!t) {
        return 1;
    }
    if (t->nil) {
        return n == t->nil;
    }
    return n == NULL;
}

void avl_tree_insert(tree_impl *t, void *key, void *value);
void avl_tree_delete(tree_impl *t, void *key);

void rb_tree_insert(tree_impl *t, void *key, void *value);
void rb_tree_delete(tree_impl *t, void *key);

#endif // TREE_INTERNAL_H
