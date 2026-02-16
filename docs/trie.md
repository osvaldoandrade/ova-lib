# Trie (Prefix Tree)

ova-lib provides a trie data structure for efficient storage and lookup of strings, prefix matching, and simple autocomplete style queries.

The trie stores keys as null-terminated byte strings (`const char *`). Each terminal node can store an associated `void *` value.

## API

Public declarations live in `include/trie.h`:

```c
trie* create_trie(void);
void trie_free(trie *t);

void trie_insert(trie *t, const char *word, void *value);
void* trie_search(const trie *t, const char *word);

bool trie_starts_with(const trie *t, const char *prefix);
list* trie_get_words_with_prefix(const trie *t, const char *prefix);

bool trie_delete(trie *t, const char *word);

int trie_count_words(const trie *t);
int trie_count_prefixes(const trie *t, const char *prefix);
```

## Semantics

- Ownership: the trie allocates/frees internal nodes only. Stored values are pointers and are never copied or freed.
- `trie_insert`: creates nodes as needed. Inserting an existing word updates its value.
- `trie_search`: returns the value pointer for an exact word match, or `NULL` if not found.
  - If you intentionally store `NULL` values, `trie_search` cannot distinguish between "not found" and "found with NULL value".
- `trie_starts_with`: returns `true` when at least one word exists with the given prefix.
- `trie_get_words_with_prefix`: returns a `list*` of heap-allocated `char*` words that start with the prefix.
  - The caller must free each returned string, then free the list container (`list->free(list)`).
- `trie_delete`: removes the word if present and frees unused nodes; returns `true` only when a word was removed.
- `trie_count_words`: total words stored.
- `trie_count_prefixes`: number of stored words that start with the given prefix.

## Complexity

- Insert/search/delete: `O(m)` where `m` is the string length.
- Prefix queries: `O(m + k)` where `k` is the number of matching words (to enumerate them).

## Example

```c
#include "trie.h"

int main(void) {
    trie *t = create_trie();

    int v1 = 1, v2 = 2;
    trie_insert(t, "car", &v1);
    trie_insert(t, "cat", &v2);

    int *found = (int *)trie_search(t, "car"); /* -> &v1 */

    trie_free(t);
    return found && *found == 1 ? 0 : 1;
}
```

