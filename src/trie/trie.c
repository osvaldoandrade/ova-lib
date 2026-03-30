#include "../../include/trie.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define TRIE_ALPHABET_SIZE 256

typedef struct trie_node {
    struct trie_node *children[TRIE_ALPHABET_SIZE];
    bool is_end;
    void *value;
    size_t subtree_words;
    unsigned int child_count;
} trie_node;

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

static trie_node *trie_walk(const trie *self, const char *s) {
    trie_impl *impl = trie_impl_from_self(self);
    if (!impl || !impl->root || !s) {
        return NULL;
    }

    trie_node *node = impl->root;
    const unsigned char *p = (const unsigned char *)s;
    while (*p) {
        trie_node *child = node->children[*p];
        if (!child) {
            return NULL;
        }
        node = child;
        p++;
    }
    return node;
}

static void trie_insert_method(trie *self, const char *word, void *value) {
    trie_impl *impl = trie_impl_from_self(self);
    if (!impl || !impl->root || !word) {
        return;
    }

    trie_node *node = impl->root;
    const unsigned char *p = (const unsigned char *)word;
    while (*p) {
        unsigned char c = *p;
        if (!node->children[c]) {
            trie_node *created = trie_node_create();
            if (!created) {
                return;
            }
            node->children[c] = created;
            node->child_count++;
        }
        node = node->children[c];
        p++;
    }

    if (node->is_end) {
        node->value = value;
        return;
    }

    node->is_end = true;
    node->value = value;
    impl->word_count++;

    node = impl->root;
    node->subtree_words++;
    p = (const unsigned char *)word;
    while (*p) {
        node = node->children[*p];
        if (!node) {
            break;
        }
        node->subtree_words++;
        p++;
    }
}

static void *trie_search_method(const trie *self, const char *word) {
    trie_node *node = trie_walk(self, word);
    if (!node || !node->is_end) {
        return NULL;
    }
    return node->value;
}

static bool trie_starts_with_method(const trie *self, const char *prefix) {
    trie_node *node = trie_walk(self, prefix);
    return node ? (node->subtree_words > 0) : false;
}

static int trie_count_words_method(const trie *self) {
    trie_impl *impl = trie_impl_from_self(self);
    return impl ? (int)impl->word_count : 0;
}

static int trie_count_prefixes_method(const trie *self, const char *prefix) {
    trie_node *node = trie_walk(self, prefix);
    return node ? (int)node->subtree_words : 0;
}

static void trie_collect_words(trie_node *node, char **buffer, size_t *cap, size_t len, list *out) {
    if (!node || !buffer || !*buffer || !cap || !out) {
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

    trie_node *node = trie_walk(self, prefix);
    int count = node ? (int)node->subtree_words : 0;
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
    trie_collect_words(node, &buffer, &cap, prefix_len, out);
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

    return (!is_root && !node->is_end && node->child_count == 0);
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
