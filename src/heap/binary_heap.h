#ifndef BINARY_HEAP2_H
#define BINARY_HEAP2_H

#include "../../include/heap.h"

typedef struct binary_heap {
    void **data;        // Array to hold the heap elements
    int capacity;       // Maximum number of elements in heap
    int size;           // Current number of elements in heap
    comparator cmp;     // Comparator function to define max-heap or min-heap behavior
} binary_heap;


/**
 * @brief Creates a binary heap.
 *
 * This function creates a binary heap and returns a pointer to it.
 *
 * @param maxHeap Determines if the heap should be a maximum heap (if `true`) or a minimum heap (if `false`).
 * @param initial_capacity The initial capacity of the heap. The heap will automatically resize if necessary.
 *
 * @return A pointer to the created binary heap.
 */
heap *create_binary_heap(int initial_capacity, comparator compare_function);

#endif // BINARY_HEAP2_H
