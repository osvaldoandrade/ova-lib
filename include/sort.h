#ifndef SORT_H
#define SORT_H

/**
 * @file sort.h
 * @brief Sorting, searching, and utility operations on lists.
 *
 * The sorter provides sort, shuffle, reverse, binary search, swap, min/max,
 * and copy operations that act on a list through a user-supplied comparator.
 *
 * Example Usage:
 * @code
 *   list *l = create_list(ARRAY_LIST, 10, NULL);
 *   // ... insert elements ...
 *   sorter *s = create_sorter(l, my_comparator);
 *   s->sort(s, l);
 *   void *found = s->binary_search(s, l, &target);
 *   free(s);
 * @endcode
 */

#include "list.h"

/**
 * @brief Sorter structure providing sorting and utility operations on lists.
 */
typedef struct sorter {
    comparator cmp; /**< Comparator function used for ordering. */

    /**
     * @brief Sorts the list in ascending order according to the comparator.
     *
     * @param self The sorter instance.
     * @param lst The list to sort.
     */
    void (*sort)(struct sorter* self, list* lst);

    /**
     * @brief Randomly shuffles the elements of the list.
     *
     * @param self The sorter instance.
     * @param lst The list to shuffle.
     */
    void (*shuffle)(struct sorter* self, list* lst);

    /**
     * @brief Reverses the order of elements in the list.
     *
     * @param self The sorter instance.
     * @param lst The list to reverse.
     */
    void (*reverse)(struct sorter* self, list* lst);

    /**
     * @brief Searches for an item in a sorted list using binary search.
     *
     * The list must be sorted according to the comparator before calling.
     *
     * @param self The sorter instance.
     * @param lst The sorted list to search.
     * @param item Pointer to the item to search for.
     * @return The index of the item if found, or a negative value if not found.
     */
    int (*binary_search)(struct sorter* self, list* lst, void* item);

    /**
     * @brief Swaps two elements in the list by index.
     *
     * @param self The sorter instance.
     * @param lst The list containing the elements to swap.
     * @param i Index of the first element.
     * @param j Index of the second element.
     */
    void (*swap)(struct sorter* self, list* lst, int i, int j);

    /**
     * @brief Finds both the minimum and maximum elements of the list.
     *
     * @param self The sorter instance.
     * @param lst The list to search.
     * @param[out] min Pointer that will receive the minimum element.
     * @param[out] max Pointer that will receive the maximum element.
     */
    void (*min_max)(struct sorter *self, list *lst, void **min, void **max);

    /**
     * @brief Returns the minimum element of the list.
     *
     * @param self The sorter instance.
     * @param lst The list to search.
     * @return Pointer to the minimum element, or NULL if the list is empty.
     */
    void *(*min)(struct sorter* self, list* lst);

    /**
     * @brief Returns the maximum element of the list.
     *
     * @param self The sorter instance.
     * @param lst The list to search.
     * @return Pointer to the maximum element, or NULL if the list is empty.
     */
    void *(*max)(struct sorter* self, list* lst);

    /**
     * @brief Copies all elements from the source list to the destination list.
     *
     * @param self The sorter instance.
     * @param src The source list.
     * @param dest The destination list.
     */
    void (*copy)(struct sorter* self, list* src, list* dest);
} sorter;

/**
 * @brief Creates a new sorter for the given list and comparator.
 *
 * @param data The list to be operated on (used for initialization).
 * @param cmp Comparator function used for sorting and searching.
 * @return A pointer to the newly created sorter, or NULL on allocation failure.
 */
sorter* create_sorter(list *data, comparator cmp);

#endif // SORT_H
