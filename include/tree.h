#ifndef TREE_H
#define TREE_H

#include "list.h"
#include "types.h"

typedef enum {
    TREE_AVL,
    TREE_RED_BLACK
} tree_type;

typedef struct tree tree;

/**
 * @brief Creates a balanced binary search tree.
 *
 * The tree stores key/value pointers provided by the caller. Keys are ordered
 * using the provided comparator. Keys are considered equal when cmp(a,b) == 0;
 * inserting an existing key updates its value.
 *
 * @param type Balanced tree variant to use.
 * @param cmp Comparator for keys (required).
 * @return A pointer to the created tree, or NULL on allocation failure or if cmp is NULL.
 */
tree *create_tree(tree_type type, comparator cmp);

/**
 * @brief Frees the tree and all internal node allocations.
 *
 * This function does not free user-provided keys/values.
 */
void tree_free(tree *t);

void tree_insert(tree *t, void *key, void *value);
void *tree_search(tree *t, void *key);
void tree_delete(tree *t, void *key);

/**
 * @brief Returns the value associated with the minimum key, or NULL if empty.
 */
void *tree_min(tree *t);

/**
 * @brief Returns the value associated with the maximum key, or NULL if empty.
 */
void *tree_max(tree *t);

/**
 * @brief Returns the value associated with the predecessor of @p key.
 *
 * The predecessor is the largest key strictly smaller than @p key. The key does
 * not need to exist in the tree.
 */
void *tree_predecessor(tree *t, void *key);

/**
 * @brief Returns the value associated with the successor of @p key.
 *
 * The successor is the smallest key strictly larger than @p key. The key does
 * not need to exist in the tree.
 */
void *tree_successor(tree *t, void *key);

/**
 * @brief Returns values whose keys are within the inclusive range [low, high].
 *
 * The returned list contains pointers to the values stored in the tree (it does
 * not allocate/copy keys or values). The caller owns the list container and must
 * free it with list->free(list).
 */
list *tree_range_query(tree *t, void *low, void *high);

/**
 * @brief Traverses the tree in ascending key order.
 *
 * The callback receives (key, value) pointers as stored in the tree.
 */
void tree_in_order_traverse(tree *t, void (*callback)(void *, void *));

/**
 * @brief Returns the number of key/value pairs stored in the tree.
 */
int tree_size(const tree *t);

#endif // TREE_H

