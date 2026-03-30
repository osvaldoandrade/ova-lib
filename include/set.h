#ifndef SET_H
#define SET_H

/**
 * @file set.h
 * @brief Set data structure with hash-based and tree-based implementations.
 *
 * Provides set operations including add, remove, contains, union, intersection,
 * difference, and subset checking.
 */

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
 *
 * @param s The set to free.
 */
void set_free(set *s);

/**
 * @brief Adds an element to the set.
 *
 * If the element already exists (as determined by the comparator), the set is unchanged.
 *
 * @param s The set.
 * @param element Pointer to the element to add.
 * @return true if the element was added, false if it was already present.
 */
bool set_add(set *s, void *element);

/**
 * @brief Checks whether an element is in the set.
 *
 * @param s The set.
 * @param element Pointer to the element to search for.
 * @return true if the element is found, false otherwise.
 */
bool set_contains(const set *s, void *element);

/**
 * @brief Removes an element from the set.
 *
 * @param s The set.
 * @param element Pointer to the element to remove.
 * @return true if the element was found and removed, false otherwise.
 */
bool set_remove(set *s, void *element);

/**
 * @brief Returns the number of elements in the set.
 *
 * @param s The set.
 * @return The element count.
 */
int set_size(const set *s);

/**
 * @brief Returns a new set containing all elements from both sets.
 *
 * The caller owns the returned set and must free it with set_free().
 *
 * @param a The first set.
 * @param b The second set.
 * @return A new set representing A ∪ B.
 *
 * @see set_intersection, set_difference
 */
set *set_union(const set *a, const set *b);

/**
 * @brief Returns a new set containing elements present in both sets.
 *
 * The caller owns the returned set and must free it with set_free().
 *
 * @param a The first set.
 * @param b The second set.
 * @return A new set representing A ∩ B.
 *
 * @see set_union, set_difference
 */
set *set_intersection(const set *a, const set *b);

/**
 * @brief Returns a new set containing elements in @p a but not in @p b.
 *
 * The caller owns the returned set and must free it with set_free().
 *
 * @param a The first set.
 * @param b The second set.
 * @return A new set representing A \\ B.
 *
 * @see set_union, set_intersection
 */
set *set_difference(const set *a, const set *b);

/**
 * @brief Checks whether @p a is a subset of @p b.
 *
 * @param a The potential subset.
 * @param b The potential superset.
 * @return true if every element of @p a is also in @p b, false otherwise.
 */
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
 *
 * @param s The set to convert.
 * @return A list of element pointers, or NULL on failure.
 */
list *set_to_list(const set *s);

/**
 * @brief Add multiple elements to the set.
 *
 * This function adds all elements from the given array to the set.
 * Duplicate elements (as determined by the set's comparator) are skipped.
 *
 * @param s A pointer to the set structure.
 * @param elements An array of pointers to the elements to be added.
 * @param count The number of elements to add.
 */
void set_add_bulk(set *s, void **elements, int count);

#endif // SET_H

