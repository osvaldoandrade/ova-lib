#ifndef SORT_H
#define SORT_H

#include "list.h"
#include "types.h"

/**
 * @brief Public sorter object.
 *
 * Concrete storage details live in @p impl.
 */
typedef struct sorter {
    void *impl;
    void *user_data; /**< User-provided context pointer. */

    /**
     * @brief Sort a list in place.
     *
     * @param self Sorter instance.
     * @param lst List to sort.
     */
    void (*sort)(struct sorter *self, list *lst);

    /**
     * @brief Shuffle a list in place.
     *
     * @param self Sorter instance.
     * @param lst List to shuffle.
     */
    void (*shuffle)(struct sorter *self, list *lst);

    /**
     * @brief Reverse a list in place.
     *
     * @param self Sorter instance.
     * @param lst List to reverse.
     */
    void (*reverse)(struct sorter *self, list *lst);

    /**
     * @brief Run binary search on a sorted list.
     *
     * @param self Sorter instance.
     * @param lst Sorted list.
     * @param item Item to search for.
     * @return Matching index, or -1 when missing.
     */
    int (*binary_search)(struct sorter *self, list *lst, void *item);

    /**
     * @brief Swap two list positions.
     *
     * @param self Sorter instance.
     * @param lst Target list.
     * @param i First index.
     * @param j Second index.
     */
    void (*swap)(struct sorter *self, list *lst, int i, int j);

    /**
     * @brief Return both the minimum and maximum list items.
     *
     * @param self Sorter instance.
     * @param lst Target list.
     * @param min Output location for the minimum item.
     * @param max Output location for the maximum item.
     */
    void (*min_max)(struct sorter *self, list *lst, void **min, void **max);

    /**
     * @brief Return the minimum list item.
     *
     * @param self Sorter instance.
     * @param lst Target list.
     * @return Minimum payload pointer, or NULL when the list is empty.
     */
    void *(*min)(struct sorter *self, list *lst);

    /**
     * @brief Return the maximum list item.
     *
     * @param self Sorter instance.
     * @param lst Target list.
     * @return Maximum payload pointer, or NULL when the list is empty.
     */
    void *(*max)(struct sorter *self, list *lst);

    /**
     * @brief Copy list items from one list to another.
     *
     * @param self Sorter instance.
     * @param src Source list.
     * @param dest Destination list.
     */
    void (*copy)(struct sorter *self, list *src, list *dest);

    /**
     * @brief Release the sorter and its internal allocations.
     *
     * @param self Sorter instance.
     */
    void (*free)(struct sorter *self);
} sorter;

/**
 * @brief Create a new sorter.
 *
 * @param cmp Comparator used by the sorting operations.
 * @return New sorter instance, or NULL on failure.
 */
sorter *create_sorter(comparator cmp);

#endif // SORT_H
