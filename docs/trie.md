# Trie

The trie stores null-terminated byte strings and associates each terminal word with one `void *` value. The alphabet size in the current implementation is `256`, so the structure works on raw byte strings rather than only on letters.

## Core API

```c
trie *create_trie(void);
void trie_free(trie *t);

void trie_insert(trie *t, const char *word, void *value);
void *trie_search(const trie *t, const char *word);

bool trie_starts_with(const trie *t, const char *prefix);
list *trie_get_words_with_prefix(const trie *t, const char *prefix);

bool trie_delete(trie *t, const char *word);

int trie_count_words(const trie *t);
int trie_count_prefixes(const trie *t, const char *prefix);
```

## Semantics

`trie_insert` creates missing nodes and overwrites the stored value when the word already exists. `trie_search` returns the stored value for an exact match. If you intentionally store a `NULL` value, that return path is indistinguishable from "word not found."

`trie_starts_with` reports whether at least one stored word shares the prefix. `trie_count_words` reports total stored words. `trie_count_prefixes` reports how many stored words share the prefix.

`trie_delete` removes one word and prunes unused nodes on the path back up the tree.

## Prefix Result Ownership

`trie_get_words_with_prefix` returns an allocated list container filled with heap-allocated `char *` copies of the matching words. The caller must free each returned string and then free the list container.
