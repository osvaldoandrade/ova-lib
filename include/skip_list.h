#ifndef SKIP_LIST_H
#define SKIP_LIST_H

#include "types.h"

/**
 * @file skip_list.h
 * @brief Probabilistic skip list – an alternative to balanced trees.
 *
 * A skip list maintains multiple layers of sorted linked lists.  Each
 * higher layer acts as an "express lane" for the layer below, yielding
 * expected O(log n) search, insertion and deletion without the need for
 * rotations required by AVL or red-black trees.
 */

/**
 * @brief Public skip-list object.
 *
 * Concrete storage details live in @p impl.
 */
typedef struct skip_list {
    void *impl;
    void *user_data; /**< User-provided context pointer. */

    /**
     * @brief Insert or update a key/value pair.
     *
     * If the key already exists its value is replaced.
     *
     * @param self Skip list instance.
     * @param key  Key pointer.
     * @param value Value pointer.
     * @return OVA_SUCCESS on success, or a negative ova_error_code on failure.
     */
    ova_error_code (*insert)(struct skip_list *self, void *key, void *value);

    /**
     * @brief Search for a key and return its value.
     *
     * @param self Skip list instance.
     * @param key  Key pointer.
     * @return Stored value pointer, or NULL when missing.
     */
    void *(*search)(struct skip_list *self, void *key);

    /**
     * @brief Delete a key/value pair by key.
     *
     * @param self Skip list instance.
     * @param key  Key pointer to remove.
     * @return OVA_SUCCESS on success, or a negative ova_error_code on failure.
     */
    ova_error_code (*delete)(struct skip_list *self, void *key);

    /**
     * @brief Return the number of stored key/value pairs.
     *
     * @param self Skip list instance.
     * @return Number of stored pairs.
     */
    int (*size)(const struct skip_list *self);

    /**
     * @brief Release the skip list and its internal allocations.
     *
     * The skip list does not free user keys or values.
     *
     * @param self Skip list instance.
     */
    void (*free)(struct skip_list *self);
} skip_list;

/**
 * @brief Create a new skip list.
 *
 * @param max_level Maximum number of levels (a good default is 16).
 * @param cmp       Comparator for keys.
 * @return New skip list instance, or NULL on failure.
 */
skip_list *create_skip_list(int max_level, comparator cmp);

#endif /* SKIP_LIST_H */
