#ifndef LIST_H
#define LIST_H

#include "types.h"

typedef enum {
    ARRAY_LIST,
    LINKED_LIST,
    SORTED_LIST
} ListType;

/**
 * @brief Public list object.
 *
 * The list stores user-owned payload pointers. Concrete storage details live in
 * @p impl.
 */
typedef struct list {
    void *impl;

    /** @internal List variant used for cloning. */
    ListType _type;
    /** @internal Comparator used for sorted-list cloning. */
    comparator _cmp;

    /**
     * @brief Insert an item at the given index.
     *
     * @param self List instance.
     * @param item Item pointer to insert.
     * @param index Zero-based insertion index.
     * @return OVA_SUCCESS on success, or a negative ova_error_code on failure.
     */
    ova_error_code (*insert)(struct list *self, void *item, int index);

    /**
     * @brief Insert multiple items at the current end of the list.
     *
     * @param self List instance.
     * @param elements Array of payload pointers.
     * @param count Number of elements in @p elements.
     * @return OVA_SUCCESS on success, or a negative ova_error_code on failure.
     */
    ova_error_code (*insert_bulk)(struct list *self, void **elements, int count);

    /**
     * @brief Retrieve the item at the given index.
     *
     * @param self List instance.
     * @param index Zero-based item index.
     * @return Stored payload pointer, or NULL when out of bounds.
     */
    void *(*get)(struct list *self, int index);

    /**
     * @brief Remove the item at the given index.
     *
     * @param self List instance.
     * @param index Zero-based item index.
     * @return OVA_SUCCESS on success, or a negative ova_error_code on failure.
     */
    ova_error_code (*remove)(struct list *self, int index);

    /**
     * @brief Return the current element count.
     *
     * @param self List instance.
     * @return Number of stored elements.
     */
    int (*size)(const struct list *self);

    /**
     * @brief Release the list and its internal allocations.
     *
     * The list does not free user payloads.
     *
     * @param self List instance.
     */
    void (*free)(struct list *self);

    /**
     * @brief Create a shallow copy of the list.
     *
     * Copies the list structure but shares element pointers with the original.
     *
     * @param self List instance.
     * @return New list instance, or NULL on failure.
     */
    struct list *(*clone_shallow)(const struct list *self);

    /**
     * @brief Create a deep copy of the list.
     *
     * Copies the list structure and each element using the provided copier.
     *
     * @param self List instance.
     * @param copier Function used to duplicate each element.
     * @return New list instance, or NULL on failure.
     */
    struct list *(*clone_deep)(const struct list *self, element_copier copier);
} list;

/**
 * @brief Create a new list.
 *
 * @param type List backend to construct.
 * @param initial_capacity Initial capacity when applicable.
 * @param cmp Comparator used by SORTED_LIST. Ignored for other variants.
 * @return New list instance, or NULL on failure.
 */
list *create_list(ListType type, int initial_capacity, comparator cmp);

#endif // LIST_H
