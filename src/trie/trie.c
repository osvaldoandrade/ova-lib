#include "../../include/trie.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define TRIE_ALPHABET_SIZE 256
#define TRIE_SSO_MAX 15

typedef struct trie_node {
    struct trie_node *children[TRIE_ALPHABET_SIZE];
    bool is_end;
    void *value;
    size_t subtree_words;
    unsigned int child_count;
    unsigned char sso_len;
    char sso_suffix[TRIE_SSO_MAX + 1];
} trie_node;

typedef enum {
    WALK_EXACT,
    WALK_SSO_FULL,
    WALK_SSO_PREFIX,
    WALK_MISS
} walk_result_t;

typedef struct trie_impl {
    trie_node *root;
    size_t word_count;
} trie_impl;

static trie_impl *trie_impl_from_self(const trie *self) {
    return self ? (trie_impl *)self->impl : NULL;
}

static trie_node *trie_node_create(void) {
    return (trie_node *)calloc(1, sizeof(trie_node));
}

static void trie_node_free(trie_node *node) {
    if (!node) {
        return;
    }

    for (int i = 0; i < TRIE_ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            trie_node_free(node->children[i]);
            node->children[i] = NULL;
        }
    }
    free(node);
}

/**
 * Expand an SSO-compressed suffix into a chain of regular child nodes.
 * Returns the leaf node at the end of the expanded chain, or NULL on failure.
 */
static trie_node *sso_expand(trie_node *node) {
    if (!node || node->sso_len == 0) {
        return node;
    }

    char suffix[TRIE_SSO_MAX + 1];
    unsigned char len = node->sso_len;
    memcpy(suffix, node->sso_suffix, len);
    suffix[len] = '\0';
    bool end = node->is_end;
    void *val = node->value;

    node->sso_len = 0;
    node->sso_suffix[0] = '\0';
    node->is_end = false;
    node->value = NULL;

    trie_node *cur = node;
    for (unsigned char i = 0; i < len; i++) {
        unsigned char c = (unsigned char)suffix[i];
        trie_node *child = trie_node_create();
        if (!child) {
            return NULL;
        }
        cur->children[c] = child;
        cur->child_count++;
        child->subtree_words = 1;
        cur = child;
    }

    cur->is_end = end;
    cur->value = val;
    return cur;
}

/**
 * SSO-aware walk. Returns a walk_result_t describing how the walk ended.
 * out_node: the deepest node reached.
 * out_sso_consumed: how many SSO suffix chars were consumed by the search string.
 */
static walk_result_t trie_walk_ex(const trie *self, const char *s,
                                  trie_node **out_node,
                                  size_t *out_sso_consumed) {
    trie_impl *impl = trie_impl_from_self(self);
    *out_node = NULL;
    if (out_sso_consumed) {
        *out_sso_consumed = 0;
    }
    if (!impl || !impl->root || !s) {
        return WALK_MISS;
    }

    trie_node *node = impl->root;
    const unsigned char *p = (const unsigned char *)s;
    while (*p) {
        if (node->sso_len > 0) {
            size_t remaining = strlen((const char *)p);
            if (remaining <= (size_t)node->sso_len &&
                memcmp(node->sso_suffix, p, remaining) == 0) {
                *out_node = node;
                if (out_sso_consumed) {
                    *out_sso_consumed = remaining;
                }
                return (remaining == (size_t)node->sso_len)
                           ? WALK_SSO_FULL
                           : WALK_SSO_PREFIX;
            }
            return WALK_MISS;
        }
        trie_node *child = node->children[*p];
        if (!child) {
            return WALK_MISS;
        }
        node = child;
        p++;
    }

    *out_node = node;
    return WALK_EXACT;
}

static ova_error_code trie_insert_method(trie *self, const char *word, void *value) {
    trie_impl *impl = trie_impl_from_self(self);
    if (!impl || !impl->root || !word) {
        return OVA_ERROR_INVALID_ARG;
    }

    /* First pass: descend to the insertion point, recording the path length. */
    size_t depth = 0;
    const unsigned char *scan = (const unsigned char *)word;
    while (*scan) { depth++; scan++; }

    /* Allocate path buffer to avoid a second root-to-leaf walk for
       subtree_words updates. Depth + 2 covers root and SSO-created nodes. */
    trie_node **path = (trie_node **)malloc((depth + 2) * sizeof(trie_node *));
    if (!path) {
        return OVA_ERROR_MEMORY;
    }
    size_t path_len = 0;

    trie_node *node = impl->root;
    path[path_len++] = node;
    const unsigned char *p = (const unsigned char *)word;
    while (*p) {
        if (node->sso_len > 0) {
            if (!sso_expand(node)) {
                free(path);
                return OVA_ERROR_MEMORY;
            }
        }

        unsigned char c = *p;
        if (!node->children[c]) {
            trie_node *created = trie_node_create();
            if (!created) {
                free(path);
                return OVA_ERROR_MEMORY;
            }
            node->children[c] = created;
            node->child_count++;

            size_t remaining = strlen((const char *)(p + 1));
            if (remaining > 0 && remaining <= TRIE_SSO_MAX) {
                memcpy(created->sso_suffix, p + 1, remaining);
                created->sso_suffix[remaining] = '\0';
                created->sso_len = (unsigned char)remaining;
                created->is_end = true;
                created->value = value;
                impl->word_count++;

                path[path_len++] = created;
                for (size_t i = 0; i < path_len; i++) {
                    path[i]->subtree_words++;
                }
                free(path);
                return OVA_SUCCESS;
            }
        }
        node = node->children[c];
        path[path_len++] = node;
        p++;
    }

    if (node->sso_len > 0) {
        if (!sso_expand(node)) {
            free(path);
            return OVA_ERROR_MEMORY;
        }
    }

    if (node->is_end) {
        node->value = value;
        free(path);
        return OVA_SUCCESS;
    }

    node->is_end = true;
    node->value = value;
    impl->word_count++;

    for (size_t i = 0; i < path_len; i++) {
        path[i]->subtree_words++;
    }
    free(path);
    return OVA_SUCCESS;
}

static void *trie_search_method(const trie *self, const char *word) {
    trie_node *node;
    walk_result_t r = trie_walk_ex(self, word, &node, NULL);
    if (r == WALK_SSO_FULL) {
        return node->is_end ? node->value : NULL;
    }
    if (r == WALK_EXACT) {
        return (node->is_end && node->sso_len == 0) ? node->value : NULL;
    }
    return NULL;
}

static bool trie_starts_with_method(const trie *self, const char *prefix) {
    trie_node *node;
    walk_result_t r = trie_walk_ex(self, prefix, &node, NULL);
    if (r == WALK_MISS) {
        return false;
    }
    return node->subtree_words > 0;
}

static int trie_count_words_method(const trie *self) {
    trie_impl *impl = trie_impl_from_self(self);
    return impl ? (int)impl->word_count : 0;
}

static int trie_count_prefixes_method(const trie *self, const char *prefix) {
    trie_node *node;
    walk_result_t r = trie_walk_ex(self, prefix, &node, NULL);
    if (r == WALK_MISS) {
        return 0;
    }
    return (int)node->subtree_words;
}

static void trie_collect_words(trie_node *node, char **buffer, size_t *cap, size_t len, list *out) {
    if (!node || !buffer || !*buffer || !cap || !out) {
        return;
    }

    if (node->sso_len > 0) {
        if (node->is_end) {
            size_t total = len + (size_t)node->sso_len;
            if (total + 1 > *cap) {
                size_t new_cap = (*cap == 0) ? 64 : (*cap * 2);
                while (total + 1 > new_cap) {
                    new_cap *= 2;
                }
                char *new_buf = (char *)realloc(*buffer, new_cap);
                if (!new_buf) {
                    return;
                }
                *buffer = new_buf;
                *cap = new_cap;
            }
            memcpy(*buffer + len, node->sso_suffix, node->sso_len);
            char *word = (char *)malloc(total + 1);
            if (word) {
                memcpy(word, *buffer, total);
                word[total] = '\0';
                out->insert(out, word, out->size(out));
            }
        }
        return;
    }

    if (node->is_end) {
        char *word = (char *)malloc(len + 1);
        if (word) {
            memcpy(word, *buffer, len);
            word[len] = '\0';
            out->insert(out, word, out->size(out));
        }
    }

    for (int i = 0; i < TRIE_ALPHABET_SIZE; i++) {
        trie_node *child = node->children[i];
        if (!child) {
            continue;
        }

        if (len + 2 > *cap) {
            size_t new_cap = (*cap == 0) ? 64 : (*cap * 2);
            while (len + 2 > new_cap) {
                new_cap *= 2;
            }
            char *new_buf = (char *)realloc(*buffer, new_cap);
            if (!new_buf) {
                return;
            }
            *buffer = new_buf;
            *cap = new_cap;
        }

        (*buffer)[len] = (char)i;
        trie_collect_words(child, buffer, cap, len + 1, out);
    }
}

static list *trie_get_words_with_prefix_method(const trie *self, const char *prefix) {
    trie_impl *impl = trie_impl_from_self(self);
    if (!impl || !impl->root || !prefix) {
        return NULL;
    }

    trie_node *node;
    size_t sso_consumed = 0;
    walk_result_t r = trie_walk_ex(self, prefix, &node, &sso_consumed);
    int count = 0;
    if (r != WALK_MISS && node) {
        count = (int)node->subtree_words;
    }

    list *out = create_list(ARRAY_LIST, count > 0 ? count : 4, NULL);
    if (!out) {
        return NULL;
    }

    if (!node || count == 0) {
        return out;
    }

    size_t prefix_len = strlen(prefix);
    size_t cap = prefix_len + 32;
    if (cap < 64) {
        cap = 64;
    }

    char *buffer = (char *)malloc(cap);
    if (!buffer) {
        out->free(out);
        return NULL;
    }

    memcpy(buffer, prefix, prefix_len);

    if (r == WALK_EXACT) {
        trie_collect_words(node, &buffer, &cap, prefix_len, out);
    } else if (r == WALK_SSO_FULL) {
        if (node->is_end) {
            char *word = (char *)malloc(prefix_len + 1);
            if (word) {
                memcpy(word, prefix, prefix_len);
                word[prefix_len] = '\0';
                out->insert(out, word, out->size(out));
            }
        }
    } else if (r == WALK_SSO_PREFIX) {
        if (node->is_end) {
            size_t remaining_sso = (size_t)node->sso_len - sso_consumed;
            size_t word_len = prefix_len + remaining_sso;
            if (word_len + 1 > cap) {
                size_t new_cap = word_len + 32;
                char *new_buf = (char *)realloc(buffer, new_cap);
                if (new_buf) {
                    buffer = new_buf;
                    cap = new_cap;
                }
            }
            memcpy(buffer + prefix_len, node->sso_suffix + sso_consumed, remaining_sso);
            char *word = (char *)malloc(word_len + 1);
            if (word) {
                memcpy(word, buffer, word_len);
                word[word_len] = '\0';
                out->insert(out, word, out->size(out));
            }
        }
    }

    free(buffer);
    return out;
}

static bool trie_delete_recursive(trie_node *node,
                                  const unsigned char *word,
                                  size_t idx,
                                  bool *removed,
                                  bool is_root) {
    if (!node || !word || !removed) {
        return false;
    }

    if (word[idx] == '\0') {
        if (node->sso_len > 0) {
            return false;
        }
        if (!node->is_end) {
            return false;
        }
        node->is_end = false;
        node->value = NULL;
        if (node->subtree_words > 0) {
            node->subtree_words--;
        }
        *removed = true;
        return (!is_root && node->child_count == 0);
    }

    if (node->sso_len > 0) {
        size_t remaining = strlen((const char *)(word + idx));
        if (remaining == (size_t)node->sso_len &&
            memcmp(node->sso_suffix, word + idx, remaining) == 0) {
            if (!node->is_end) {
                return false;
            }
            node->is_end = false;
            node->value = NULL;
            node->sso_len = 0;
            node->sso_suffix[0] = '\0';
            if (node->subtree_words > 0) {
                node->subtree_words--;
            }
            *removed = true;
            return (!is_root && node->child_count == 0);
        }
        return false;
    }

    unsigned char c = word[idx];
    trie_node *child = node->children[c];
    if (!child) {
        return false;
    }

    bool should_free_child = trie_delete_recursive(child, word, idx + 1, removed, false);
    if (!*removed) {
        return false;
    }

    if (node->subtree_words > 0) {
        node->subtree_words--;
    }

    if (should_free_child) {
        trie_node_free(child);
        node->children[c] = NULL;
        if (node->child_count > 0) {
            node->child_count--;
        }
    }

    return (!is_root && !node->is_end && node->child_count == 0 && node->sso_len == 0);
}

static bool trie_delete_method(trie *self, const char *word) {
    trie_impl *impl = trie_impl_from_self(self);
    if (!impl || !impl->root || !word) {
        return false;
    }

    bool removed = false;
    (void)trie_delete_recursive(impl->root, (const unsigned char *)word, 0, &removed, true);
    if (removed && impl->word_count > 0) {
        impl->word_count--;
    }
    return removed;
}

static void trie_free_method(trie *self) {
    if (!self) {
        return;
    }

    trie_impl *impl = trie_impl_from_self(self);
    if (impl) {
        trie_node_free(impl->root);
        impl->root = NULL;
        free(impl);
        self->impl = NULL;
    }

    free(self);
}

trie *create_trie(void) {
    trie *out = (trie *)calloc(1, sizeof(trie));
    if (!out) {
        return NULL;
    }

    trie_impl *impl = (trie_impl *)calloc(1, sizeof(trie_impl));
    if (!impl) {
        free(out);
        return NULL;
    }

    impl->root = trie_node_create();
    if (!impl->root) {
        free(impl);
        free(out);
        return NULL;
    }

    impl->word_count = 0;

    out->impl = impl;
    out->insert = trie_insert_method;
    out->search = trie_search_method;
    out->starts_with = trie_starts_with_method;
    out->get_words_with_prefix = trie_get_words_with_prefix_method;
    out->delete = trie_delete_method;
    out->count_words = trie_count_words_method;
    out->count_prefixes = trie_count_prefixes_method;
    out->free = trie_free_method;

    return out;
}
