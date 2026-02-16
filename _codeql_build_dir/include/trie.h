#ifndef TRIE_H
#define TRIE_H

#include "list.h"
#include "types.h"

typedef struct trie trie;

/*
 * Create a new, empty trie.
 *
 * Returns a pointer to the trie, or NULL if allocation fails.
 */
trie *create_trie(void);

/*
 * Free all internal memory used by the trie structure itself.
 *
 * Note: This does not free any user-provided value pointers stored in
 * the trie. Callers are responsible for managing the lifetime of values
 * inserted with trie_insert().
 */
void trie_free(trie *t);

/*
 * Insert a word into the trie, associating it with an arbitrary value.
 *
 * The trie stores the pointer to 'value' but does not take ownership of it
 * and will never free it. The caller must ensure that 'value' remains valid
 * for as long as it may be accessed via trie_search(), and must free it
 * when no longer needed.
 *
 * If the word already exists, its associated value is updated.
 */
void trie_insert(trie *t, const char *word, void *value);

/*
 * Look up a word in the trie and return the associated value pointer.
 *
 * Returns:
 *   - the stored value pointer if the word exists in the trie
 *   - NULL if the word is not present
 *
 * The returned pointer is the same one that was passed to trie_insert()
 * and is not owned by the trie. The caller remains responsible for
 * managing and eventually freeing the value.
 */
void *trie_search(const trie *t, const char *word);

/*
 * Check whether any word in the trie starts with the given prefix.
 *
 * Returns true if at least one word with the prefix exists, false otherwise.
 */
bool trie_starts_with(const trie *t, const char *prefix);

/*
 * Return a list of all words in the trie that start with the given prefix.
 *
 * The returned list is heap-allocated. Each element of the list is a
 * heap-allocated 'char *' containing a word. The caller owns the returned
 * list and all its strings, and must:
 *   - free each 'char *' string in the list, and then
 *   - free the list itself (see list.h for list destruction details).
 *
 * Returns NULL if allocation fails, or an empty list if no words match.
 */
list *trie_get_words_with_prefix(const trie *t, const char *prefix);

/*
 * Delete a word from the trie.
 *
 * Returns true if the word was present and removed, false otherwise.
 * Deleting a word does not free any associated value pointer; callers
 * must free values themselves if needed.
 */
bool trie_delete(trie *t, const char *word);

/*
 * Count the total number of words stored in the trie.
 *
 * If the trie contains more than INT_MAX words, returns INT_MAX.
 */
int trie_count_words(const trie *t);

/*
 * Count the number of words in the trie that start with the given prefix.
 *
 * If more than INT_MAX words match the prefix, returns INT_MAX.
 */
int trie_count_prefixes(const trie *t, const char *prefix);

#endif // TRIE_H

