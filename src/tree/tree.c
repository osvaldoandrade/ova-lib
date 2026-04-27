#include "../../include/tree.h"
#include "tree_internal.h"

#include <stdlib.h>

static tree_node *tree_search_node(const tree_impl *impl, void *key) {
    if (!impl || !impl->cmp || !key) {
        return (impl && impl->nil) ? impl->nil : NULL;
    }

    tree_node *cur = impl->root;
    tree_node *nil = impl->nil;
    while (cur && cur != nil) {
        int cmp = impl->cmp(key, cur->key);
        if (cmp == 0) {
            return cur;
        }
        cur = (cmp < 0) ? cur->left : cur->right;
    }

    return nil ? nil : NULL;
}

static tree_node *tree_min_node(const tree_impl *impl, tree_node *node) {
    if (!impl || tree_node_is_nil(impl, node)) {
        return (impl && impl->nil) ? impl->nil : NULL;
    }

    tree_node *nil = impl->nil;
    while (node->left && node->left != nil) {
        node = node->left;
    }
    return node;
}

static tree_node *tree_max_node(const tree_impl *impl, tree_node *node) {
    if (!impl || tree_node_is_nil(impl, node)) {
        return (impl && impl->nil) ? impl->nil : NULL;
    }

    tree_node *nil = impl->nil;
    while (node->right && node->right != nil) {
        node = node->right;
    }
    return node;
}

static void tree_free_node(tree_impl *impl, tree_node *node) {
    if (tree_node_is_nil(impl, node)) {
        return;
    }

    tree_free_node(impl, node->left);
    tree_free_node(impl, node->right);
    free(node);
}

static void tree_in_order_node(tree_impl *impl, tree_node *node, void (*callback)(void *, void *)) {
    if (!impl || !callback || tree_node_is_nil(impl, node)) {
        return;
    }

    tree_in_order_node(impl, node->left, callback);
    callback(node->key, node->value);
    tree_in_order_node(impl, node->right, callback);
}

static void tree_range_query_node(tree_impl *impl, tree_node *node, void *low, void *high, list *out) {
    if (!impl || !impl->cmp || !out || tree_node_is_nil(impl, node)) {
        return;
    }

    int cmp_low = impl->cmp(node->key, low);
    int cmp_high = impl->cmp(node->key, high);

    if (cmp_low > 0) {
        tree_range_query_node(impl, node->left, low, high, out);
    }

    if (cmp_low >= 0 && cmp_high <= 0) {
        out->insert(out, node->value, out->size(out));
    }

    if (cmp_high < 0) {
        tree_range_query_node(impl, node->right, low, high, out);
    }
}

static ova_error_code tree_insert_method(tree *self, void *key, void *value) {
    tree_impl *impl = tree_impl_from_tree(self);
    if (!impl || !impl->cmp || !key) {
        return OVA_ERROR_INVALID_ARG;
    }

    if (impl->type == TREE_AVL) {
        avl_tree_insert(impl, key, value);
    } else if (impl->type == TREE_RED_BLACK) {
        rb_tree_insert(impl, key, value);
    }
    return OVA_SUCCESS;
}

static void *tree_search_method(tree *self, void *key) {
    tree_impl *impl = tree_impl_from_tree(self);
    if (!impl || !impl->cmp || !key) {
        return NULL;
    }

    tree_node *node = tree_search_node(impl, key);
    if (!node || tree_node_is_nil(impl, node)) {
        return NULL;
    }
    return node->value;
}

static ova_error_code tree_delete_method(tree *self, void *key) {
    tree_impl *impl = tree_impl_from_tree(self);
    if (!impl || !impl->cmp || !key) {
        return OVA_ERROR_INVALID_ARG;
    }

    if (impl->type == TREE_AVL) {
        avl_tree_delete(impl, key);
    } else if (impl->type == TREE_RED_BLACK) {
        rb_tree_delete(impl, key);
    }
    return OVA_SUCCESS;
}

static void *tree_min_method(tree *self) {
    tree_impl *impl = tree_impl_from_tree(self);
    if (!impl) {
        return NULL;
    }

    tree_node *node = tree_min_node(impl, impl->root);
    if (!node || tree_node_is_nil(impl, node)) {
        return NULL;
    }
    return node->value;
}

static void *tree_max_method(tree *self) {
    tree_impl *impl = tree_impl_from_tree(self);
    if (!impl) {
        return NULL;
    }

    tree_node *node = tree_max_node(impl, impl->root);
    if (!node || tree_node_is_nil(impl, node)) {
        return NULL;
    }
    return node->value;
}

static void *tree_predecessor_method(tree *self, void *key) {
    tree_impl *impl = tree_impl_from_tree(self);
    if (!impl || !impl->cmp || !key) {
        return NULL;
    }

    tree_node *nil = impl->nil;
    tree_node *cur = impl->root;
    tree_node *best = nil ? nil : NULL;

    while (cur && cur != nil) {
        int cmp = impl->cmp(key, cur->key);
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

static void *tree_successor_method(tree *self, void *key) {
    tree_impl *impl = tree_impl_from_tree(self);
    if (!impl || !impl->cmp || !key) {
        return NULL;
    }

    tree_node *nil = impl->nil;
    tree_node *cur = impl->root;
    tree_node *best = nil ? nil : NULL;

    while (cur && cur != nil) {
        int cmp = impl->cmp(key, cur->key);
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

static list *tree_range_query_method(tree *self, void *low, void *high) {
    tree_impl *impl = tree_impl_from_tree(self);
    if (!impl || !impl->cmp || !low || !high) {
        return NULL;
    }

    if (impl->cmp(low, high) > 0) {
        return NULL;
    }

    int cap = (impl->size > 0 && impl->size < 1024u * 1024u) ? (int)impl->size : 4;
    list *out = create_list(ARRAY_LIST, cap, NULL);
    if (!out) {
        return NULL;
    }
    tree_range_query_node(impl, impl->root, low, high, out);
    return out;
}

static void tree_in_order_traverse_method(tree *self, void (*callback)(void *, void *)) {
    tree_impl *impl = tree_impl_from_tree(self);
    if (!impl || !callback) {
        return;
    }
    tree_in_order_node(impl, impl->root, callback);
}

static int tree_size_method(const tree *self) {
    tree_impl *impl = tree_impl_from_tree(self);
    return impl ? (int)impl->size : 0;
}

static void tree_free_method(tree *self) {
    if (!self) {
        return;
    }

    tree_impl *impl = tree_impl_from_tree(self);
    if (impl) {
        if (impl->type == TREE_RED_BLACK) {
            if (impl->root && impl->nil && impl->root != impl->nil) {
                tree_free_node(impl, impl->root);
            }
            free(impl->nil);
            impl->nil = NULL;
        } else {
            tree_free_node(impl, impl->root);
        }

        free(impl);
        self->impl = NULL;
    }

    free(self);
}

tree *create_tree(tree_type type, comparator cmp) {
    if (!cmp) {
        return NULL;
    }

    tree *out = (tree *)calloc(1, sizeof(tree));
    if (!out) {
        return NULL;
    }

    tree_impl *impl = (tree_impl *)calloc(1, sizeof(tree_impl));
    if (!impl) {
        free(out);
        return NULL;
    }

    impl->type = type;
    impl->cmp = cmp;
    impl->root = NULL;
    impl->nil = NULL;
    impl->size = 0;

    if (type == TREE_RED_BLACK) {
        impl->nil = (tree_node *)calloc(1, sizeof(tree_node));
        if (!impl->nil) {
            free(impl);
            free(out);
            return NULL;
        }
        impl->nil->color = RB_BLACK;
        impl->nil->left = impl->nil;
        impl->nil->right = impl->nil;
        impl->nil->parent = impl->nil;
        impl->nil->height = 0;
        impl->root = impl->nil;
    } else if (type == TREE_AVL) {
        impl->root = NULL;
    } else {
        free(impl);
        free(out);
        return NULL;
    }

    out->impl = impl;
    out->insert = tree_insert_method;
    out->search = tree_search_method;
    out->delete = tree_delete_method;
    out->min = tree_min_method;
    out->max = tree_max_method;
    out->predecessor = tree_predecessor_method;
    out->successor = tree_successor_method;
    out->range_query = tree_range_query_method;
    out->in_order_traverse = tree_in_order_traverse_method;
    out->size = tree_size_method;
    out->free = tree_free_method;

    return out;
}
