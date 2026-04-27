#ifndef HEAP_H
#define HEAP_H

/**
 * @file heap.h
 * @brief Heap data structure with binary and Fibonacci heap implementations.
 *
 * Provides a common heap interface with pluggable implementations including
 * binary heap and Fibonacci heap. Operations are accessed via function pointers.
 */

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
     * @return OVA_SUCCESS on success, or a negative ova_error_code on failure.
     */
    ova_error_code (*put)(struct heap *self, void *item);

    /**
     * @brief Put an item into the heap and return a handle to it.
     *
     * This function pointer is used to insert an item into the heap and return
     * an opaque handle that can be used with decrease_key or delete_node.
     *
     * @param self A pointer to the heap structure.
     * @param item A pointer to the item to be inserted.
     * @return An opaque handle to the inserted node, or NULL on failure.
     */
    void *(*put_with_handle)(struct heap *self, void *item);

    /**
     * @brief Decrease the key of a node in the heap.
     *
     * This function pointer is used to increase the priority of a node identified by
     * the given handle. Despite the name "decrease_key" (from algorithm literature),
     * this operation increases the node's priority according to the heap's comparator.
     * 
     * For a min-heap: the new value should be less than the current value.
     * For a max-heap: the new value should be greater than the current value.
     *
     * @param self A pointer to the heap structure.
     * @param node_handle An opaque handle to the node (returned by put_with_handle).
     * @param new_value A pointer to the new value for the node (must increase priority).
     * @return OVA_SUCCESS on success, or a negative ova_error_code on failure.
     */
    ova_error_code (*decrease_key)(struct heap *self, void *node_handle, void *new_value);

    /**
     * @brief Delete a node from the heap.
     *
     * This function pointer is used to delete a node identified by the given handle
     * from the heap.
     *
     * @param self A pointer to the heap structure.
     * @param node_handle An opaque handle to the node (returned by put_with_handle).
     * @return OVA_SUCCESS on success, or a negative ova_error_code on failure.
     */
    ova_error_code (*delete_node)(struct heap *self, void *node_handle);

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
     * @brief Remove all elements without destroying the heap.
     *
     * The heap becomes empty but retains its allocated capacity.
     * User payloads are not freed.
     *
     * @param self A pointer to the heap structure.
     */
    void (*clear)(struct heap *self);

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
 * @param type The type of heap to be created (e.g., BINARY_HEAP, FIBONACCI_HEAP).
 * @param capacity Initial capacity (used by binary heaps, ignored by Fibonacci heaps).
 * @param compare_function Comparator function used to order heap elements.
 * @return A pointer to the newly created heap.
 *         If the specified heap type is invalid, NULL is returned.
 */
heap *create_heap(HeapType type, int capacity, comparator compare_function);

#endif // HEAP_H
