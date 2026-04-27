#ifndef SET_H
#define SET_H

#include "list.h"
#include "types.h"

typedef enum {
    SET_HASH,
    SET_TREE
} set_type;

/**
 * @brief Public set object.
 *
 * Concrete storage details live in @p impl.
 */
typedef struct set {
    void *impl;
    void *user_data; /**< User-provided context pointer. */

    /**
     * @brief Add an element to the set.
     *
     * @param self Set instance.
     * @param element Payload pointer to add.
     * @return true when the element was inserted, false when it was already present or invalid.
     */
    bool (*add)(struct set *self, void *element);

    /**
     * @brief Add multiple elements to the set.
     *
     * @param self Set instance.
     * @param elements Array of payload pointers.
     * @param count Number of elements in @p elements.
     */
    void (*add_bulk)(struct set *self, void **elements, int count);

    /**
     * @brief Check whether an element is present.
     *
     * @param self Set instance.
     * @param element Payload pointer to query.
     * @return true when present, false otherwise.
     */
    bool (*contains)(const struct set *self, void *element);

    /**
     * @brief Remove an element from the set.
     *
     * @param self Set instance.
     * @param element Payload pointer to remove.
     * @return true when removed, false when missing or invalid.
     */
    bool (*remove)(struct set *self, void *element);

    /**
     * @brief Return the number of stored elements.
     *
     * @param self Set instance.
     * @return Number of stored elements.
     */
    int (*size)(const struct set *self);

    /**
     * @brief Create a new set containing the union of two compatible sets.
     *
     * @param self Left-hand set instance.
     * @param other Right-hand set instance.
     * @return New set instance, or NULL on incompatibility / allocation failure.
     */
    struct set *(*union_with)(const struct set *self, const struct set *other);

    /**
     * @brief Create a new set containing the intersection of two compatible sets.
     *
     * @param self Left-hand set instance.
     * @param other Right-hand set instance.
     * @return New set instance, or NULL on incompatibility / allocation failure.
     */
    struct set *(*intersection_with)(const struct set *self, const struct set *other);

    /**
     * @brief Create a new set containing the difference of two compatible sets.
     *
     * @param self Left-hand set instance.
     * @param other Right-hand set instance.
     * @return New set instance, or NULL on incompatibility / allocation failure.
     */
    struct set *(*difference_with)(const struct set *self, const struct set *other);

    /**
     * @brief Check whether this set is a subset of another compatible set.
     *
     * @param self Left-hand set instance.
     * @param other Right-hand set instance.
     * @return true when every element in @p self is present in @p other.
     */
    bool (*is_subset_of)(const struct set *self, const struct set *other);

    /**
     * @brief Convert the set to a list of stored elements.
     *
     * The returned list container is owned by the caller.
     *
     * @param self Set instance.
     * @return New list of stored payload pointers.
     */
    list *(*to_list)(const struct set *self);

    /**
     * @brief Release the set and its internal allocations.
     *
     * The set does not free user payloads.
     *
     * @param self Set instance.
     */
    void (*free)(struct set *self);
} set;

/**
 * @brief Create a new set.
 *
 * For SET_HASH, both @p cmp and @p hash must be provided, or both may be NULL
 * to use default pointer-identity semantics. For SET_TREE, @p hash is ignored.
 *
 * @param type Set backend to construct.
 * @param cmp Comparator for set elements.
 * @param hash Hash function for SET_HASH.
 * @return New set instance, or NULL on failure.
 */
set *create_set(set_type type, comparator cmp, hash_func_t hash);

#endif // SET_H
