/**
 * @file heap.h
 * @brief Public API for heap data structures.
 */

#ifndef HEAP_H
#define HEAP_H

#include "types.h"

typedef enum {
    BINARY_HEAP,
    FIBONACCI_HEAP
} HeapType;

typedef struct heap {

    void *impl;

    /**
     * @brief Put an item into the heap.
     *
     * This function pointer is used to insert an item into the heap.
     *
     * @param self A pointer to the heap structure.
     * @param item A pointer to the item to be inserted.
     */
    void (*put)(struct heap *self, void *item);

    /**
     * @brief Extract the top element from the heap.
     *
     * This function pointer is used to extract the top element (such as maximum or minimum,
     * depending on the heap type) from the heap.
     *
     * @param self A pointer to the heap structure.
     * @return A pointer to the extracted item.
     */
    void *(*pop)(struct heap *self);

    /**
     * @brief Get the top element of the heap without removing it.
     *
     * This function pointer is used to peek at the top element of the heap without removing it.
     *
     * @param self A pointer to the heap structure.
     * @return A pointer to the top element.
     */
    void *(*peek)(const struct heap *self);

    /**
     * @brief Get the number of elements in the heap.
     *
     * This function pointer is used to get the current size of the heap.
     *
     * @param self A pointer to the heap structure.
     * @return The number of elements in the heap.
     */
    int (*size)(const struct heap *self);

    /**
     * @brief Free the memory allocated for the heap.
     *
     * This function pointer is used to free all the memory allocated for the heap, including the heap structure itself.
     *
     * @param self A pointer to the heap structure.
     */
    void (*free)(struct heap *self);

} heap;

/**
 * @brief Create a new heap.
 *
 * This function creates a new heap based on the specified heap type.
 *
 * @param type The type of heap to be created (e.g., BINARY_HEAP, FIBONACCI_HEAP, BINOMIAL_HEAP).
 * @return A pointer to the newly created heap.
 *         If the specified heap type is invalid, NULL is returned.
 */
heap *create_heap(HeapType type, int capacity, comparator compare_function);

#endif // HEAP_H
