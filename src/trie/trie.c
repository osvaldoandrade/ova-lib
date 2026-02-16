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

    /* Number of words that end in this node's subtree (including this node). */
    size_t subtree_words;

    /* Number of non-NULL children; helps efficient deletion cleanup. */
    unsigned int child_count;
} trie_node;

struct trie {
    trie_node *root;
    size_t word_count;
};

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

trie *create_trie(void) {
    trie *t = (trie *)calloc(1, sizeof(trie));
    if (!t) {
        return NULL;
    }

    t->root = trie_node_create();
    if (!t->root) {
        free(t);
        return NULL;
    }

    t->word_count = 0;
    return t;
}

void trie_free(trie *t) {
    if (!t) {
        return;
    }
    trie_node_free(t->root);
    t->root = NULL;
    free(t);
}

static trie_node *trie_walk(const trie *t, const char *s) {
    if (!t || !t->root || !s) {
        return NULL;
    }

    trie_node *node = t->root;
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

void trie_insert(trie *t, const char *word, void *value) {
    if (!t || !t->root || !word) {
        return;
    }

    trie_node *node = t->root;
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
    t->word_count++;

    /* Update subtree word counts along the path (including root). */
    node = t->root;
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

void *trie_search(const trie *t, const char *word) {
    trie_node *node = trie_walk(t, word);
    if (!node || !node->is_end) {
        return NULL;
    }
    return node->value;
}

bool trie_starts_with(const trie *t, const char *prefix) {
    trie_node *node = trie_walk(t, prefix);
    if (!node) {
        return false;
    }
    return node->subtree_words > 0;
}

int trie_count_words(const trie *t) {
    if (!t) {
        return 0;
    }
    return (int)t->word_count;
}

int trie_count_prefixes(const trie *t, const char *prefix) {
    trie_node *node = trie_walk(t, prefix);
    if (!node) {
        return 0;
    }
    return (int)node->subtree_words;
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

list *trie_get_words_with_prefix(const trie *t, const char *prefix) {
    if (!t || !t->root || !prefix) {
        return NULL;
    }

    trie_node *node = trie_walk(t, prefix);
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

bool trie_delete(trie *t, const char *word) {
    if (!t || !t->root || !word) {
        return false;
    }

    bool removed = false;
    (void)trie_delete_recursive(t->root, (const unsigned char *)word, 0, &removed, true);
    if (removed && t->word_count > 0) {
        t->word_count--;
    }
    return removed;
}

