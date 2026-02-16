#ifndef TRIE_H
#define TRIE_H

#include "list.h"
#include "types.h"

typedef struct trie trie;

trie *create_trie(void);
void trie_free(trie *t);

void trie_insert(trie *t, const char *word, void *value);
void *trie_search(const trie *t, const char *word);

bool trie_starts_with(const trie *t, const char *prefix);
list *trie_get_words_with_prefix(const trie *t, const char *prefix);

bool trie_delete(trie *t, const char *word);

int trie_count_words(const trie *t);
int trie_count_prefixes(const trie *t, const char *prefix);

#endif // TRIE_H

