#ifndef LIST_H
#define LIST_H
#include "types.h"

typedef enum {
    ARRAY_LIST,
    LINKED_LIST,
    SORTED_LIST
} ListType;


typedef struct list {

    void *impl;

    /**
     * @brief Function pointer to insert an item in the list at a specific index.
     *
     * This function pointer is used to insert an item into the list at a specified index.
     *
     * @param self A pointer to the list structure.
     * @param item A pointer to the item to be inserted.
     * @param index The index where the item should be inserted.
     */
    void (*insert)(struct list *self, void *item, int index);

    /**
     * @brief Function pointer to retrieve an item from the list.
     *
     * This function pointer is used to retrieve an item from the list at the specified index.
     * The returned item will be a pointer to the data at the given index.
     *
     * @param self A pointer to the list structure.
     * @param index The index of the item to retrieve.
     * @return A pointer to the data at the specified index.
     */
    void *(*get)(struct list *self, int index);

    /**
     * @brief Remove an item from the list at the specified index.
     *
     * This function pointer is used to remove an item from the list at a specified index.
     * The item at the given index will be removed from the list and the remaining items will be shifted to fill the gap.
     *
     * @param self A pointer to the list structure.
     * @param index The index of the item to be removed.
     */
    void (*remove)(struct list *self, int index);

    /**
     * @brief Function pointer to retrieve the size of the list.
     *
     * This function pointer is used to retrieve the size of the list, i.e., the number of elements in the list.
     * The function returns an integer value representing the size of the list.
     *
     * @param self A pointer to the list structure.
     * @return An integer value representing the size of the list.
     */
    int (*size)(const struct list *self);

    /**
     * @brief Function pointer to free the memory allocated for the list.
     *
     * This function pointer is used to free the memory allocated for the list.
     * It is important to free the memory to avoid memory leaks.
     *
     * @param self A pointer to the list structure.
     */
    void (*free)(struct list *self);

} list;

/**
 * @brief Create a new list.
 *
 * This function creates a new list based on the specified list type and initial capacity.
 * The list can be of type ARRAY_LIST, LINKED_LIST, or SORTED_LIST.
 *
 * @param type The type of list to be created.
 * @param initial_capacity The initial capacity of the list when applicable.
 * @param cmp Comparator used to keep elements ordered for SORTED_LIST.
 *            It is ignored for other list types and may be NULL in those cases.
 * @return A pointer to the newly created list.
 *         If the specified list type is invalid, NULL is returned.
 */
list *create_list(ListType type, int initial_capacity, comparator cmp);

#endif // LIST_H
