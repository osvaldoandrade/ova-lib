#include "tree_internal.h"

#include <stdlib.h>

static int avl_max_int(int a, int b) {
    return a > b ? a : b;
}

static int avl_height(const tree_node *n) {
    return n ? n->height : 0;
}

static void avl_update_height(tree_node *n) {
    if (!n) {
        return;
    }
    n->height = 1 + avl_max_int(avl_height(n->left), avl_height(n->right));
}

static int avl_balance(const tree_node *n) {
    if (!n) {
        return 0;
    }
    return avl_height(n->left) - avl_height(n->right);
}

static tree_node *avl_new_node(void *key, void *value, tree_node *parent) {
    tree_node *n = (tree_node *)calloc(1, sizeof(tree_node));
    if (!n) {
        return NULL;
    }
    n->key = key;
    n->value = value;
    n->left = NULL;
    n->right = NULL;
    n->parent = parent;
    n->height = 1;
    n->color = RB_BLACK;
    return n;
}

static tree_node *avl_rotate_right(tree_node *y) {
    tree_node *x = y ? y->left : NULL;
    if (!x) {
        return y;
    }
    tree_node *t2 = x->right;

    x->right = y;
    x->parent = y->parent;

    y->left = t2;
    y->parent = x;

    if (t2) {
        t2->parent = y;
    }

    avl_update_height(y);
    avl_update_height(x);
    return x;
}

static tree_node *avl_rotate_left(tree_node *x) {
    tree_node *y = x ? x->right : NULL;
    if (!y) {
        return x;
    }
    tree_node *t2 = y->left;

    y->left = x;
    y->parent = x->parent;

    x->right = t2;
    x->parent = y;

    if (t2) {
        t2->parent = x;
    }

    avl_update_height(x);
    avl_update_height(y);
    return y;
}

static tree_node *avl_insert_node(tree *t, tree_node *node, void *key, void *value, tree_node *parent) {
    if (!node) {
        tree_node *created = avl_new_node(key, value, parent);
        if (created) {
            t->size++;
        }
        return created;
    }

    int cmp = t->cmp(key, node->key);
    if (cmp == 0) {
        node->value = value;
        return node;
    }

    if (cmp < 0) {
        node->left = avl_insert_node(t, node->left, key, value, node);
        if (node->left) {
            node->left->parent = node;
        }
    } else {
        node->right = avl_insert_node(t, node->right, key, value, node);
        if (node->right) {
            node->right->parent = node;
        }
    }

    avl_update_height(node);
    int balance = avl_balance(node);

    if (balance > 1 && node->left && t->cmp(key, node->left->key) < 0) {
        return avl_rotate_right(node);
    }

    if (balance < -1 && node->right && t->cmp(key, node->right->key) > 0) {
        return avl_rotate_left(node);
    }

    if (balance > 1 && node->left && t->cmp(key, node->left->key) > 0) {
        node->left = avl_rotate_left(node->left);
        if (node->left) {
            node->left->parent = node;
        }
        return avl_rotate_right(node);
    }

    if (balance < -1 && node->right && t->cmp(key, node->right->key) < 0) {
        node->right = avl_rotate_right(node->right);
        if (node->right) {
            node->right->parent = node;
        }
        return avl_rotate_left(node);
    }

    return node;
}

static tree_node *avl_min_node(tree_node *node) {
    if (!node) {
        return NULL;
    }
    while (node->left) {
        node = node->left;
    }
    return node;
}

static tree_node *avl_delete_node(tree *t, tree_node *root, void *key) {
    if (!root) {
        return NULL;
    }

    int cmp = t->cmp(key, root->key);
    if (cmp < 0) {
        root->left = avl_delete_node(t, root->left, key);
        if (root->left) {
            root->left->parent = root;
        }
    } else if (cmp > 0) {
        root->right = avl_delete_node(t, root->right, key);
        if (root->right) {
            root->right->parent = root;
        }
    } else {
        if (!root->left || !root->right) {
            tree_node *child = root->left ? root->left : root->right;
            if (!child) {
                free(root);
                t->size--;
                return NULL;
            }

            child->parent = root->parent;
            free(root);
            t->size--;
            return child;
        }

        tree_node *succ = avl_min_node(root->right);
        if (succ) {
            root->key = succ->key;
            root->value = succ->value;
            root->right = avl_delete_node(t, root->right, succ->key);
            if (root->right) {
                root->right->parent = root;
            }
        }
    }

    avl_update_height(root);
    int balance = avl_balance(root);

    if (balance > 1) {
        if (avl_balance(root->left) >= 0) {
            return avl_rotate_right(root);
        }
        root->left = avl_rotate_left(root->left);
        if (root->left) {
            root->left->parent = root;
        }
        return avl_rotate_right(root);
    }

    if (balance < -1) {
        if (avl_balance(root->right) <= 0) {
            return avl_rotate_left(root);
        }
        root->right = avl_rotate_right(root->right);
        if (root->right) {
            root->right->parent = root;
        }
        return avl_rotate_left(root);
    }

    return root;
}

void avl_tree_insert(tree *t, void *key, void *value) {
    if (!t || !t->cmp) {
        return;
    }

    t->root = avl_insert_node(t, t->root, key, value, NULL);
    if (t->root) {
        t->root->parent = NULL;
    }
}

void avl_tree_delete(tree *t, void *key) {
    if (!t || !t->cmp || !key) {
        return;
    }

    t->root = avl_delete_node(t, t->root, key);
    if (t->root) {
        t->root->parent = NULL;
    }
}

