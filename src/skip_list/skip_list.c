#include "../../include/skip_list.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ------------------------------------------------------------------ */
/*  Internal types                                                     */
/* ------------------------------------------------------------------ */

typedef struct skip_node {
    void *key;
    void *value;
    int level;                 /* highest level this node participates in */
    struct skip_node *forward[1]; /* flexible tail: levels 0..level (co-allocated) */
} skip_node;

typedef struct {
    skip_node *header;   /* dummy head node (sentinel)               */
    int max_level;       /* maximum allowed level (0-based)           */
    int level;           /* current highest level in the list         */
    int size;            /* number of stored key/value pairs          */
    comparator cmp;
    unsigned int seed;   /* per-instance PRNG seed for level generation */
} skip_list_impl;

/* ------------------------------------------------------------------ */
/*  Helpers                                                            */
/* ------------------------------------------------------------------ */

static skip_list_impl *impl_from(const skip_list *self) {
    return self ? (skip_list_impl *)self->impl : NULL;
}

static unsigned int next_random(skip_list_impl *sl) {
    /* xorshift32: better bit quality than the LCG variant for coin flips,
     * and just as cheap. Each bit is independent with p≈0.5. */
    unsigned int x = sl->seed;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    sl->seed = x ? x : 1u;
    return sl->seed;
}

/**
 * Create a node with @p lvl+1 forward pointers (levels 0..lvl).
 * The node header and the forward[] tail are allocated in a single
 * contiguous block to halve malloc/free traffic on the insert path.
 */
static skip_node *create_node(int lvl, void *key, void *value) {
    /* skip_node already reserves space for forward[1]; ask for the
     * extra (lvl) slots beyond that one. */
    size_t bytes = sizeof(skip_node) + (size_t)lvl * sizeof(skip_node *);
    skip_node *n = (skip_node *)malloc(bytes);
    if (!n) {
        return NULL;
    }
    for (int i = 0; i <= lvl; i++) {
        n->forward[i] = NULL;
    }
    n->key   = key;
    n->value = value;
    n->level = lvl;
    return n;
}

/**
 * Generate a random level using a coin-flip approach (p = 0.5).
 * Single PRNG call: count trailing zero bits of the random word.
 * For a uniformly random 32-bit word w, P(ctz(w) >= k) = 2^-k, which
 * is exactly the geometric distribution the coin-flip loop produces.
 */
static int random_level(skip_list_impl *sl) {
    unsigned int r = next_random(sl);
    int lvl;
#if defined(__GNUC__) || defined(__clang__)
    /* __builtin_ctz is undefined for 0; xorshift never returns 0,
     * but guard anyway in case future RNG changes. */
    lvl = r ? __builtin_ctz(r) : 32;
#else
    lvl = 0;
    while ((r & 1u) && lvl < 32) {
        lvl++;
        r >>= 1;
    }
#endif
    if (lvl > sl->max_level) {
        lvl = sl->max_level;
    }
    return lvl;
}

/* ------------------------------------------------------------------ */
/*  Skip list operations                                               */
/* ------------------------------------------------------------------ */

static ova_error_code sl_insert(skip_list *self, void *key, void *value) {
    skip_list_impl *sl = impl_from(self);
    if (!sl || !sl->cmp || !key) {
        return OVA_ERROR_INVALID_ARG;
    }

    /* update[i] will point to the last node at level i whose key < key */
    skip_node *update_stack[32];
    skip_node **update;
    if (sl->max_level + 1 <= 32) {
        update = update_stack;
        memset(update, 0, (size_t)(sl->max_level + 1) * sizeof(skip_node *));
    } else {
        update = (skip_node **)calloc((size_t)(sl->max_level + 1),
                                      sizeof(skip_node *));
        if (!update) {
            return OVA_ERROR_MEMORY;
        }
    }

    skip_node *cur = sl->header;
    for (int i = sl->level; i >= 0; i--) {
        while (cur->forward[i] && sl->cmp(cur->forward[i]->key, key) < 0) {
            cur = cur->forward[i];
        }
        update[i] = cur;
    }

    cur = cur->forward[0];

    /* Key already exists – update value */
    if (cur && sl->cmp(cur->key, key) == 0) {
        cur->value = value;
        if (update != update_stack) {
            free(update);
        }
        return OVA_SUCCESS;
    }

    int new_level = random_level(sl);

    /* If new level exceeds current, update the extra levels */
    if (new_level > sl->level) {
        for (int i = sl->level + 1; i <= new_level; i++) {
            update[i] = sl->header;
        }
        sl->level = new_level;
    }

    skip_node *new_node = create_node(new_level, key, value);
    if (!new_node) {
        if (update != update_stack) {
            free(update);
        }
        return OVA_ERROR_MEMORY;
    }

    for (int i = 0; i <= new_level; i++) {
        new_node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = new_node;
    }

    sl->size++;
    if (update != update_stack) {
        free(update);
    }
    return OVA_SUCCESS;
}

static void *sl_search(skip_list *self, void *key) {
    skip_list_impl *sl = impl_from(self);
    if (!sl || !sl->cmp || !key) {
        return NULL;
    }

    skip_node *cur = sl->header;
    for (int i = sl->level; i >= 0; i--) {
        while (cur->forward[i] && sl->cmp(cur->forward[i]->key, key) < 0) {
            cur = cur->forward[i];
        }
    }

    cur = cur->forward[0];
    if (cur && sl->cmp(cur->key, key) == 0) {
        return cur->value;
    }
    return NULL;
}

static ova_error_code sl_delete(skip_list *self, void *key) {
    skip_list_impl *sl = impl_from(self);
    if (!sl || !sl->cmp || !key) {
        return OVA_ERROR_INVALID_ARG;
    }

    skip_node *update_stack[32];
    skip_node **update;
    if (sl->max_level + 1 <= 32) {
        update = update_stack;
        memset(update, 0, (size_t)(sl->max_level + 1) * sizeof(skip_node *));
    } else {
        update = (skip_node **)calloc((size_t)(sl->max_level + 1),
                                      sizeof(skip_node *));
        if (!update) {
            return OVA_ERROR_MEMORY;
        }
    }

    skip_node *cur = sl->header;
    for (int i = sl->level; i >= 0; i--) {
        while (cur->forward[i] && sl->cmp(cur->forward[i]->key, key) < 0) {
            cur = cur->forward[i];
        }
        update[i] = cur;
    }

    cur = cur->forward[0];

    if (!cur || sl->cmp(cur->key, key) != 0) {
        if (update != update_stack) {
            free(update);
        }
        return OVA_ERROR_NOT_FOUND;
    }

    for (int i = 0; i <= sl->level; i++) {
        if (update[i]->forward[i] != cur) {
            break;
        }
        update[i]->forward[i] = cur->forward[i];
    }

    free(cur);
    sl->size--;

    /* Lower the current level if the top levels are now empty */
    while (sl->level > 0 && sl->header->forward[sl->level] == NULL) {
        sl->level--;
    }

    if (update != update_stack) {
        free(update);
    }
    return OVA_SUCCESS;
}

static int sl_size(const skip_list *self) {
    skip_list_impl *sl = impl_from(self);
    return sl ? sl->size : 0;
}

static void sl_free(skip_list *self) {
    if (!self) {
        return;
    }

    skip_list_impl *sl = impl_from(self);
    if (sl) {
        skip_node *cur = sl->header;
        while (cur) {
            skip_node *next = cur->forward[0];
            free(cur);
            cur = next;
        }
        free(sl);
        self->impl = NULL;
    }
    free(self);
}

/* ------------------------------------------------------------------ */
/*  Public constructor                                                 */
/* ------------------------------------------------------------------ */

skip_list *create_skip_list(int max_level, comparator cmp) {
    if (!cmp || max_level < 1) {
        return NULL;
    }

    skip_list *out = (skip_list *)calloc(1, sizeof(skip_list));
    if (!out) {
        return NULL;
    }

    skip_list_impl *sl = (skip_list_impl *)calloc(1, sizeof(skip_list_impl));
    if (!sl) {
        free(out);
        return NULL;
    }

    /* max_level is stored as 0-based internally (max index) */
    sl->max_level = max_level - 1;
    sl->level     = 0;
    sl->size      = 0;
    sl->cmp       = cmp;
    {
        static unsigned int seed_counter;
        sl->seed = (unsigned int)time(NULL) ^ (unsigned int)(size_t)sl
                   ^ (++seed_counter * 2654435761u);
        if (sl->seed == 0u) sl->seed = 1u;
    }

    sl->header = create_node(sl->max_level, NULL, NULL);
    if (!sl->header) {
        free(sl);
        free(out);
        return NULL;
    }

    out->impl   = sl;
    out->insert = sl_insert;
    out->search = sl_search;
    out->delete = sl_delete;
    out->size   = sl_size;
    out->free   = sl_free;

    return out;
}
