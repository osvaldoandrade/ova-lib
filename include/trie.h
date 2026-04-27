#ifndef TRIE_H
#define TRIE_H

#include "list.h"
#include "types.h"

/**
 * @brief Public trie object.
 *
 * Concrete storage details live in @p impl.
 */
typedef struct trie {
    void *impl;

    /**
     * @brief Insert or update a word/value pair.
     *
     * @param self Trie instance.
     * @param word NUL-terminated key string.
     * @param value Value pointer to store.
     * @return OVA_SUCCESS on success, or a negative ova_error_code on failure.
     */
    ova_error_code (*insert)(struct trie *self, const char *word, void *value);

    /**
     * @brief Search for a word and return its value.
     *
     * @param self Trie instance.
     * @param word NUL-terminated key string.
     * @return Stored value pointer, or NULL when missing.
     */
    void *(*search)(const struct trie *self, const char *word);

    /**
     * @brief Check whether any word starts with the given prefix.
     *
     * @param self Trie instance.
     * @param prefix NUL-terminated prefix string.
     * @return true when at least one word matches the prefix.
     */
    bool (*starts_with)(const struct trie *self, const char *prefix);

    /**
     * @brief Return all words with the given prefix.
     *
     * The returned list contains heap-allocated NUL-terminated strings and is
     * owned by the caller.
     *
     * @param self Trie instance.
     * @param prefix NUL-terminated prefix string.
     * @return New list of matching strings.
     */
    list *(*get_words_with_prefix)(const struct trie *self, const char *prefix);

    /**
     * @brief Delete a word from the trie.
     *
     * @param self Trie instance.
     * @param word NUL-terminated key string.
     * @return true when the word was removed, false otherwise.
     */
    bool (*delete)(struct trie *self, const char *word);

    /**
     * @brief Return the number of stored words.
     *
     * @param self Trie instance.
     * @return Number of stored words.
     */
    int (*count_words)(const struct trie *self);

    /**
     * @brief Return the number of stored words sharing the given prefix.
     *
     * @param self Trie instance.
     * @param prefix NUL-terminated prefix string.
     * @return Number of matching words.
     */
    int (*count_prefixes)(const struct trie *self, const char *prefix);

    /**
     * @brief Release the trie and its internal allocations.
     *
     * The trie does not free user payloads.
     *
     * @param self Trie instance.
     */
    void (*free)(struct trie *self);
} trie;

/**
 * @brief Create a new trie.
 *
 * @return New trie instance, or NULL on failure.
 */
trie *create_trie(void);

#endif // TRIE_H
