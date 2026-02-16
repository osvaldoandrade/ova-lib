#include "../../include/tree.h"
#include "tree_internal.h"

#include <stdlib.h>

static tree_node *tree_search_node(const tree *t, void *key) {
    if (!t || !t->cmp || !key) {
        return (t && t->nil) ? t->nil : NULL;
    }

    tree_node *cur = t->root;
    tree_node *nil = t->nil;
    while (cur && cur != nil) {
        int cmp = t->cmp(key, cur->key);
        if (cmp == 0) {
            return cur;
        }
        cur = (cmp < 0) ? cur->left : cur->right;
    }

    return nil ? nil : NULL;
}

static tree_node *tree_min_node(const tree *t, tree_node *node) {
    if (!t || tree_node_is_nil(t, node)) {
        return (t && t->nil) ? t->nil : NULL;
    }

    tree_node *nil = t->nil;
    while (node->left && node->left != nil) {
        node = node->left;
    }
    return node;
}

static tree_node *tree_max_node(const tree *t, tree_node *node) {
    if (!t || tree_node_is_nil(t, node)) {
        return (t && t->nil) ? t->nil : NULL;
    }

    tree_node *nil = t->nil;
    while (node->right && node->right != nil) {
        node = node->right;
    }
    return node;
}

static void tree_free_node(tree *t, tree_node *node) {
    if (tree_node_is_nil(t, node)) {
        return;
    }

    tree_free_node(t, node->left);
    tree_free_node(t, node->right);
    free(node);
}

static void tree_in_order_node(tree *t, tree_node *node, void (*callback)(void *, void *)) {
    if (!t || !callback || tree_node_is_nil(t, node)) {
        return;
    }

    tree_in_order_node(t, node->left, callback);
    callback(node->key, node->value);
    tree_in_order_node(t, node->right, callback);
}

static void tree_range_query_node(tree *t, tree_node *node, void *low, void *high, list *out) {
    if (!t || !t->cmp || !out || tree_node_is_nil(t, node)) {
        return;
    }

    int cmp_low = t->cmp(node->key, low);
    int cmp_high = t->cmp(node->key, high);

    if (cmp_low > 0) {
        tree_range_query_node(t, node->left, low, high, out);
    }

    if (cmp_low >= 0 && cmp_high <= 0) {
        out->insert(out, node->value, out->size(out));
    }

    if (cmp_high < 0) {
        tree_range_query_node(t, node->right, low, high, out);
    }
}

tree *create_tree(tree_type type, comparator cmp) {
    if (!cmp) {
        return NULL;
    }

    tree *t = (tree *)calloc(1, sizeof(tree));
    if (!t) {
        return NULL;
    }

    t->type = type;
    t->cmp = cmp;
    t->root = NULL;
    t->nil = NULL;
    t->size = 0;

    if (type == TREE_RED_BLACK) {
        t->nil = (tree_node *)calloc(1, sizeof(tree_node));
        if (!t->nil) {
            free(t);
            return NULL;
        }
        t->nil->color = RB_BLACK;
        t->nil->left = t->nil;
        t->nil->right = t->nil;
        t->nil->parent = t->nil;
        t->nil->height = 0;
        t->root = t->nil;
    } else if (type == TREE_AVL) {
        t->root = NULL;
    } else {
        free(t);
        return NULL;
    }

    return t;
}

void tree_free(tree *t) {
    if (!t) {
        return;
    }

    if (t->type == TREE_RED_BLACK) {
        if (t->root && t->nil && t->root != t->nil) {
            tree_free_node(t, t->root);
        }
        if (t->nil) {
            free(t->nil);
            t->nil = NULL;
        }
    } else {
        tree_free_node(t, t->root);
    }

    free(t);
}

void tree_insert(tree *t, void *key, void *value) {
    if (!t || !t->cmp || !key) {
        return;
    }

    if (t->type == TREE_AVL) {
        avl_tree_insert(t, key, value);
    } else if (t->type == TREE_RED_BLACK) {
        rb_tree_insert(t, key, value);
    }
}

void *tree_search(tree *t, void *key) {
    if (!t || !t->cmp || !key) {
        return NULL;
    }

    tree_node *n = tree_search_node(t, key);
    if (!n || tree_node_is_nil(t, n)) {
        return NULL;
    }
    return n->value;
}

void tree_delete(tree *t, void *key) {
    if (!t || !t->cmp || !key) {
        return;
    }

    if (t->type == TREE_AVL) {
        avl_tree_delete(t, key);
    } else if (t->type == TREE_RED_BLACK) {
        rb_tree_delete(t, key);
    }
}

void *tree_min(tree *t) {
    if (!t) {
        return NULL;
    }

    tree_node *n = tree_min_node(t, t->root);
    if (!n || tree_node_is_nil(t, n)) {
        return NULL;
    }
    return n->value;
}

void *tree_max(tree *t) {
    if (!t) {
        return NULL;
    }

    tree_node *n = tree_max_node(t, t->root);
    if (!n || tree_node_is_nil(t, n)) {
        return NULL;
    }
    return n->value;
}

void *tree_predecessor(tree *t, void *key) {
    if (!t || !t->cmp || !key) {
        return NULL;
    }

    tree_node *nil = t->nil;
    tree_node *cur = t->root;
    tree_node *best = nil ? nil : NULL;

    while (cur && cur != nil) {
        int cmp = t->cmp(key, cur->key);
        if (cmp <= 0) {
            cur = cur->left;
        } else {
            best = cur;
            cur = cur->right;
        }
    }

    if (!best || (nil && best == nil)) {
        return NULL;
    }
    return best->value;
}

void *tree_successor(tree *t, void *key) {
    if (!t || !t->cmp || !key) {
        return NULL;
    }

    tree_node *nil = t->nil;
    tree_node *cur = t->root;
    tree_node *best = nil ? nil : NULL;

    while (cur && cur != nil) {
        int cmp = t->cmp(key, cur->key);
        if (cmp >= 0) {
            cur = cur->right;
        } else {
            best = cur;
            cur = cur->left;
        }
    }

    if (!best || (nil && best == nil)) {
        return NULL;
    }
    return best->value;
}

list *tree_range_query(tree *t, void *low, void *high) {
    if (!t || !t->cmp || !low || !high) {
        return NULL;
    }

    if (t->cmp(low, high) > 0) {
        return NULL;
    }

    int cap = (t->size > 0 && t->size < 1024u * 1024u) ? (int)t->size : 4;
    list *out = create_list(ARRAY_LIST, cap, NULL);
    if (!out) {
        return NULL;
    }
    tree_range_query_node(t, t->root, low, high, out);
    return out;
}

void tree_in_order_traverse(tree *t, void (*callback)(void *, void *)) {
    if (!t || !callback) {
        return;
    }
    tree_in_order_node(t, t->root, callback);
}

int tree_size(const tree *t) {
    if (!t) {
        return 0;
    }
    return (int)t->size;
}

