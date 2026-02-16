#ifndef TREE_INTERNAL_H
#define TREE_INTERNAL_H

#include "../../include/tree.h"

typedef enum {
    RB_RED = 0,
    RB_BLACK = 1
} rb_color;

typedef struct tree_node tree_node;

struct tree_node {
    void *key;
    void *value;
    tree_node *left;
    tree_node *right;
    tree_node *parent;

    /* AVL bookkeeping */
    int height;

    /* Red-black bookkeeping */
    rb_color color;
};

struct tree {
    tree_type type;
    comparator cmp;
    tree_node *root;

    /* Non-NULL only for red-black trees; acts as a shared NIL/sentinel leaf. */
    tree_node *nil;

    size_t size;
};

static inline int tree_node_is_nil(const tree *t, const tree_node *n) {
    if (!t) {
        return 1;
    }
    if (t->nil) {
        return n == t->nil;
    }
    return n == NULL;
}

void avl_tree_insert(tree *t, void *key, void *value);
void avl_tree_delete(tree *t, void *key);

void rb_tree_insert(tree *t, void *key, void *value);
void rb_tree_delete(tree *t, void *key);

#endif // TREE_INTERNAL_H

