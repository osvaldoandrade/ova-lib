#ifndef SET_H
#define SET_H

#include "list.h"
#include "types.h"

typedef enum {
    SET_HASH, /* Hash-based set, average O(1) operations */
    SET_TREE  /* Tree-based ordered set, O(log n) operations */
} set_type;

typedef struct set set;

/**
 * @brief Create a set.
 *
 * For SET_HASH, both @p cmp and @p hash must be provided (or both NULL to use
 * default pointer-identity semantics).
 *
 * For SET_TREE, @p hash is ignored. If @p cmp is NULL, a default pointer-identity
 * comparator is used.
 *
 * @param type Set implementation type.
 * @param cmp Comparator for elements (defines uniqueness).
 * @param hash Hash function for elements (SET_HASH only).
 * @return Newly created set or NULL on failure.
 */
set *create_set(set_type type, comparator cmp, hash_func_t hash);

/**
 * @brief Free the set and all internal allocations.
 *
 * This function does not free user-provided elements.
 */
void set_free(set *s);

bool set_add(set *s, void *element);
bool set_contains(const set *s, void *element);
bool set_remove(set *s, void *element);
int set_size(const set *s);

set *set_union(const set *a, const set *b);
set *set_intersection(const set *a, const set *b);
set *set_difference(const set *a, const set *b);
bool set_is_subset(const set *a, const set *b);

/**
 * @brief Convert the set to a list of elements.
 *
 * The returned list contains pointers to the elements stored in the set
 * (no copies). The caller owns the list container and must free it with
 * list->free(list).
 *
 * For SET_TREE, the list is ordered by the set comparator (in-order traversal).
 * For SET_HASH, no ordering is guaranteed.
 */
list *set_to_list(const set *s);

#endif // SET_H

