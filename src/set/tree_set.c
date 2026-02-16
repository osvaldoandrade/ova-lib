#include "set_internal.h"

#include "../../include/tree.h"
#include "../tree/tree_internal.h"

#include <stdlib.h>

typedef struct {
    tree *t;
} tree_set_impl;

static tree_set_impl *tree_set_impl_from_set(const set *s) {
    return s ? (tree_set_impl *)s->impl : NULL;
}

static bool tree_set_add(set *s, void *element) {
    if (!s || !element) {
        return false;
    }

    tree_set_impl *impl = tree_set_impl_from_set(s);
    if (!impl || !impl->t) {
        return false;
    }

    if (tree_search(impl->t, element) != NULL) {
        return false;
    }

    int before = tree_size(impl->t);
    tree_insert(impl->t, element, element);
    return tree_size(impl->t) > before;
}

static bool tree_set_contains(const set *s, void *element) {
    if (!s || !element) {
        return false;
    }

    tree_set_impl *impl = tree_set_impl_from_set(s);
    if (!impl || !impl->t) {
        return false;
    }

    return tree_search(impl->t, element) != NULL;
}

static bool tree_set_remove(set *s, void *element) {
    if (!s || !element) {
        return false;
    }

    tree_set_impl *impl = tree_set_impl_from_set(s);
    if (!impl || !impl->t) {
        return false;
    }

    int before = tree_size(impl->t);
    tree_delete(impl->t, element);
    return tree_size(impl->t) < before;
}

static int tree_set_size(const set *s) {
    tree_set_impl *impl = tree_set_impl_from_set(s);
    if (!impl || !impl->t) {
        return 0;
    }
    return tree_size(impl->t);
}

/* Internal in-order traversal that carries the destination list via arguments. */
static void tree_set_collect_in_order(const tree *t, tree_node *node, list *out) {
    if (!t || !out || tree_node_is_nil(t, node)) {
        return;
    }

    tree_set_collect_in_order(t, node->left, out);
    out->insert(out, node->key, out->size(out));
    tree_set_collect_in_order(t, node->right, out);
}

static list *tree_set_to_list(const set *s) {
    tree_set_impl *impl = tree_set_impl_from_set(s);
    if (!impl || !impl->t) {
        return NULL;
    }

    int n = tree_size(impl->t);
    list *out = create_list(ARRAY_LIST, n > 0 ? n : 4, NULL);
    if (!out) {
        return NULL;
    }

    tree_set_collect_in_order(impl->t, impl->t->root, out);

    return out;
}

static void tree_set_destroy(set *s) {
    if (!s) {
        return;
    }

    tree_set_impl *impl = tree_set_impl_from_set(s);
    if (impl) {
        if (impl->t) {
            tree_free(impl->t);
            impl->t = NULL;
        }
        free(impl);
        s->impl = NULL;
    }
}

void *tree_set_create_impl(comparator cmp) {
    if (!cmp) {
        return NULL;
    }

    tree *t = create_tree(TREE_RED_BLACK, cmp);
    if (!t) {
        return NULL;
    }

    tree_set_impl *impl = (tree_set_impl *)calloc(1, sizeof(tree_set_impl));
    if (!impl) {
        tree_free(t);
        return NULL;
    }

    impl->t = t;
    return impl;
}

const set_ops tree_set_ops = {
        .add = tree_set_add,
        .contains = tree_set_contains,
        .remove = tree_set_remove,
        .size = tree_set_size,
        .to_list = tree_set_to_list,
        .destroy = tree_set_destroy,
};

