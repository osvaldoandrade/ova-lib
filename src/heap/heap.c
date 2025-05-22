/**
 * @file heap.c
 * @brief Factory for heap instances.
 */

#include "../../include/heap.h"
#include "binary_heap.h"
#include "fibonacci_heap.h"

/**
 * @brief Creates a new heap instance.
 *
 * This factory function wraps the concrete heap implementations and returns a
 * generic heap interface. The @p capacity parameter is only respected by the
 * binary heap variant; other implementations ignore it.
 *
 * @param type The type of heap to create.
 * @param capacity The initial capacity for a binary heap implementation.
 * @param compare_function The comparator used to order elements in the heap.
 * @return Pointer to the created heap or NULL on failure.
 */

heap *create_heap(HeapType type, int capacity, comparator compare_function) {
    switch (type) {
        case BINARY_HEAP:
            return create_binary_heap(capacity, compare_function);
        case FIBONACCI_HEAP:
            return create_fibonacci_heap(compare_function);
        default:
            return NULL;
    }
}

