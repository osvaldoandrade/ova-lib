#ifndef TREE_H
#define TREE_H

#include "list.h"
#include "types.h"

typedef enum {
    TREE_AVL,
    TREE_RED_BLACK
} tree_type;

/**
 * @brief Public balanced-tree object.
 *
 * Concrete storage details live in @p impl.
 */
typedef struct tree {
    void *impl;
    void *user_data; /**< User-provided context pointer. */

    /**
     * @brief Insert or update a key/value pair.
     *
     * @param self Tree instance.
     * @param key Key pointer.
     * @param value Value pointer.
     * @return OVA_SUCCESS on success, or a negative ova_error_code on failure.
     */
    ova_error_code (*insert)(struct tree *self, void *key, void *value);

    /**
     * @brief Search for a key and return its value.
     *
     * @param self Tree instance.
     * @param key Key pointer.
     * @return Stored value pointer, or NULL when missing.
     */
    void *(*search)(struct tree *self, void *key);

    /**
     * @brief Delete a key/value pair by key.
     *
     * @param self Tree instance.
     * @param key Key pointer to remove.
     * @return OVA_SUCCESS on success, or a negative ova_error_code on failure.
     */
    ova_error_code (*delete)(struct tree *self, void *key);

    /**
     * @brief Return the value associated with the minimum key.
     *
     * @param self Tree instance.
     * @return Value pointer, or NULL when empty.
     */
    void *(*min)(struct tree *self);

    /**
     * @brief Return the value associated with the maximum key.
     *
     * @param self Tree instance.
     * @return Value pointer, or NULL when empty.
     */
    void *(*max)(struct tree *self);

    /**
     * @brief Return the predecessor value for the given key.
     *
     * @param self Tree instance.
     * @param key Search key.
     * @return Value pointer, or NULL when no predecessor exists.
     */
    void *(*predecessor)(struct tree *self, void *key);

    /**
     * @brief Return the successor value for the given key.
     *
     * @param self Tree instance.
     * @param key Search key.
     * @return Value pointer, or NULL when no successor exists.
     */
    void *(*successor)(struct tree *self, void *key);

    /**
     * @brief Return values whose keys fall in the inclusive range [low, high].
     *
     * The returned list container is owned by the caller.
     *
     * @param self Tree instance.
     * @param low Inclusive lower bound.
     * @param high Inclusive upper bound.
     * @return New list of stored value pointers, or NULL on failure.
     */
    list *(*range_query)(struct tree *self, void *low, void *high);

    /**
     * @brief Traverse keys in ascending order.
     *
     * @param self Tree instance.
     * @param callback Callback receiving stored key/value pointers.
     */
    void (*in_order_traverse)(struct tree *self, void (*callback)(void *, void *));

    /**
     * @brief Return the number of stored key/value pairs.
     *
     * @param self Tree instance.
     * @return Number of stored pairs.
     */
    int (*size)(const struct tree *self);

    /**
     * @brief Release the tree and its internal allocations.
     *
     * The tree does not free user keys or values.
     *
     * @param self Tree instance.
     */
    void (*free)(struct tree *self);
} tree;

/**
 * @brief Create a balanced binary-search tree.
 *
 * @param type Tree backend to construct.
 * @param cmp Comparator for keys.
 * @return New tree instance, or NULL on failure.
 */
tree *create_tree(tree_type type, comparator cmp);

#endif // TREE_H
