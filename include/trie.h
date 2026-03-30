#ifndef TRIE_H
#define TRIE_H

/**
 * @file trie.h
 * @brief Trie (prefix tree) data structure for efficient string operations.
 *
 * A trie stores strings character-by-character and supports fast prefix lookups,
 * insertions, deletions, and word counting.
 *
 * Example Usage:
 * @code
 *   trie *t = create_trie();
 *   int val = 42;
 *   trie_insert(t, "hello", &val);
 *   int *result = trie_search(t, "hello");
 *   trie_free(t);
 * @endcode
 */

#include "list.h"
#include "types.h"

typedef struct trie trie;

/**
 * @brief Creates a new empty trie.
 *
 * @return A pointer to the newly created trie, or NULL on allocation failure.
 */
trie *create_trie(void);

/**
 * @brief Frees all memory associated with the trie.
 *
 * This function does not free user-provided values.
 *
 * @param t The trie to free.
 */
void trie_free(trie *t);

/**
 * @brief Inserts a word into the trie with an associated value.
 *
 * If the word already exists, its value is updated.
 *
 * @param t The trie.
 * @param word Null-terminated string to insert.
 * @param value Pointer to the value to associate with the word.
 */
void trie_insert(trie *t, const char *word, void *value);

/**
 * @brief Searches for a word in the trie.
 *
 * @param t The trie.
 * @param word Null-terminated string to search for.
 * @return Pointer to the associated value if found, or NULL if the word is not present.
 */
void *trie_search(const trie *t, const char *word);

/**
 * @brief Checks whether any word in the trie starts with the given prefix.
 *
 * @param t The trie.
 * @param prefix Null-terminated prefix string.
 * @return true if at least one word starts with @p prefix, false otherwise.
 */
bool trie_starts_with(const trie *t, const char *prefix);

/**
 * @brief Returns all words that start with the given prefix.
 *
 * The caller owns the returned list and must free it with list->free(list).
 *
 * @param t The trie.
 * @param prefix Null-terminated prefix string.
 * @return A list of null-terminated strings matching the prefix.
 */
list *trie_get_words_with_prefix(const trie *t, const char *prefix);

/**
 * @brief Deletes a word from the trie.
 *
 * @param t The trie.
 * @param word Null-terminated string to delete.
 * @return true if the word was found and deleted, false otherwise.
 */
bool trie_delete(trie *t, const char *word);

/**
 * @brief Returns the number of words stored in the trie.
 *
 * @param t The trie.
 * @return The word count.
 */
int trie_count_words(const trie *t);

/**
 * @brief Counts how many words in the trie start with the given prefix.
 *
 * @param t The trie.
 * @param prefix Null-terminated prefix string.
 * @return The number of words matching the prefix.
 */
int trie_count_prefixes(const trie *t, const char *prefix);

#endif // TRIE_H

