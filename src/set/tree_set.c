#include "set_internal.h"

#include "../../include/tree.h"

#include <stdlib.h>

typedef struct {
    tree *t;
} tree_set_impl;

static tree_set_impl *tree_set_impl_from_state(const set_impl *state) {
    return state ? (tree_set_impl *)state->backend_impl : NULL;
}

static bool tree_set_add(set_impl *state, void *element) {
    if (!state || !element) {
        return false;
    }

    tree_set_impl *impl = tree_set_impl_from_state(state);
    if (!impl || !impl->t) {
        return false;
    }

    if (impl->t->search(impl->t, element) != NULL) {
        return false;
    }

    int before = impl->t->size(impl->t);
    impl->t->insert(impl->t, element, element);
    return impl->t->size(impl->t) > before;
}

static bool tree_set_contains(const set_impl *state, void *element) {
    if (!state || !element) {
        return false;
    }

    tree_set_impl *impl = tree_set_impl_from_state(state);
    if (!impl || !impl->t) {
        return false;
    }

    return impl->t->search(impl->t, element) != NULL;
}

static bool tree_set_remove(set_impl *state, void *element) {
    if (!state || !element) {
        return false;
    }

    tree_set_impl *impl = tree_set_impl_from_state(state);
    if (!impl || !impl->t) {
        return false;
    }

    int before = impl->t->size(impl->t);
    impl->t->delete(impl->t, element);
    return impl->t->size(impl->t) < before;
}

static int tree_set_size(const set_impl *state) {
    tree_set_impl *impl = tree_set_impl_from_state(state);
    return (impl && impl->t) ? impl->t->size(impl->t) : 0;
}

static list *tree_set_to_list_out = NULL;

static void tree_set_collect_cb(void *key, void *value) {
    (void)value;
    if (!tree_set_to_list_out) {
        return;
    }
    tree_set_to_list_out->insert(tree_set_to_list_out, key, tree_set_to_list_out->size(tree_set_to_list_out));
}

static list *tree_set_to_list(const set_impl *state) {
    tree_set_impl *impl = tree_set_impl_from_state(state);
    if (!impl || !impl->t) {
        return NULL;
    }

    int n = impl->t->size(impl->t);
    list *out = create_list(ARRAY_LIST, n > 0 ? n : 4, NULL);
    if (!out) {
        return NULL;
    }

    tree_set_to_list_out = out;
    impl->t->in_order_traverse(impl->t, tree_set_collect_cb);
    tree_set_to_list_out = NULL;
    return out;
}

static void tree_set_destroy(set_impl *state) {
    if (!state) {
        return;
    }

    tree_set_impl *impl = tree_set_impl_from_state(state);
    if (impl) {
        if (impl->t) {
            impl->t->free(impl->t);
            impl->t = NULL;
        }
        free(impl);
        state->backend_impl = NULL;
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
        t->free(t);
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
