#include "tree_internal.h"

#include <stdlib.h>

static void rb_left_rotate(tree *t, tree_node *x) {
    tree_node *y = x->right;

    x->right = y->left;
    if (y->left != t->nil) {
        y->left->parent = x;
    }

    y->parent = x->parent;
    if (x->parent == t->nil) {
        t->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

static void rb_right_rotate(tree *t, tree_node *y) {
    tree_node *x = y->left;

    y->left = x->right;
    if (x->right != t->nil) {
        x->right->parent = y;
    }

    x->parent = y->parent;
    if (y->parent == t->nil) {
        t->root = x;
    } else if (y == y->parent->right) {
        y->parent->right = x;
    } else {
        y->parent->left = x;
    }

    x->right = y;
    y->parent = x;
}

static void rb_insert_fixup(tree *t, tree_node *z) {
    while (z->parent->color == RB_RED) {
        if (z->parent == z->parent->parent->left) {
            tree_node *y = z->parent->parent->right;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rb_left_rotate(t, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                rb_right_rotate(t, z->parent->parent);
            }
        } else {
            tree_node *y = z->parent->parent->left;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rb_right_rotate(t, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                rb_left_rotate(t, z->parent->parent);
            }
        }
    }
    t->root->color = RB_BLACK;
}

static tree_node *rb_search_node(const tree *t, void *key) {
    tree_node *cur = t->root;
    while (cur != t->nil) {
        int cmp = t->cmp(key, cur->key);
        if (cmp == 0) {
            return cur;
        }
        cur = (cmp < 0) ? cur->left : cur->right;
    }
    return t->nil;
}

static void rb_transplant(tree *t, tree_node *u, tree_node *v) {
    if (u->parent == t->nil) {
        t->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

static tree_node *rb_minimum(tree *t, tree_node *x) {
    while (x->left != t->nil) {
        x = x->left;
    }
    return x;
}

static void rb_delete_fixup(tree *t, tree_node *x) {
    while (x != t->root && x->color == RB_BLACK) {
        if (x == x->parent->left) {
            tree_node *w = x->parent->right;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                rb_left_rotate(t, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == RB_BLACK && w->right->color == RB_BLACK) {
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (w->right->color == RB_BLACK) {
                    w->left->color = RB_BLACK;
                    w->color = RB_RED;
                    rb_right_rotate(t, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->right->color = RB_BLACK;
                rb_left_rotate(t, x->parent);
                x = t->root;
            }
        } else {
            tree_node *w = x->parent->left;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                rb_right_rotate(t, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == RB_BLACK && w->left->color == RB_BLACK) {
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (w->left->color == RB_BLACK) {
                    w->right->color = RB_BLACK;
                    w->color = RB_RED;
                    rb_left_rotate(t, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->left->color = RB_BLACK;
                rb_right_rotate(t, x->parent);
                x = t->root;
            }
        }
    }
    x->color = RB_BLACK;
}

void rb_tree_insert(tree *t, void *key, void *value) {
    if (!t || !t->cmp || !t->nil || !key) {
        return;
    }

    tree_node *y = t->nil;
    tree_node *x = t->root;

    while (x != t->nil) {
        y = x;
        int cmp = t->cmp(key, x->key);
        if (cmp == 0) {
            x->value = value;
            return;
        }
        x = (cmp < 0) ? x->left : x->right;
    }

    tree_node *z = (tree_node *)calloc(1, sizeof(tree_node));
    if (!z) {
        return;
    }

    z->key = key;
    z->value = value;
    z->left = t->nil;
    z->right = t->nil;
    z->parent = y;
    z->color = RB_RED;
    z->height = 1;

    if (y == t->nil) {
        t->root = z;
    } else if (t->cmp(key, y->key) < 0) {
        y->left = z;
    } else {
        y->right = z;
    }

    t->size++;
    rb_insert_fixup(t, z);
}

void rb_tree_delete(tree *t, void *key) {
    if (!t || !t->cmp || !t->nil || !key) {
        return;
    }

    tree_node *z = rb_search_node(t, key);
    if (z == t->nil) {
        return;
    }

    tree_node *y = z;
    rb_color y_original_color = y->color;
    tree_node *x = t->nil;

    if (z->left == t->nil) {
        x = z->right;
        rb_transplant(t, z, z->right);
    } else if (z->right == t->nil) {
        x = z->left;
        rb_transplant(t, z, z->left);
    } else {
        y = rb_minimum(t, z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        } else {
            rb_transplant(t, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        rb_transplant(t, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    free(z);
    if (t->size > 0) {
        t->size--;
    }

    if (y_original_color == RB_BLACK) {
        rb_delete_fixup(t, x);
    }
}

