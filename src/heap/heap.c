#include "../../include/heap.h"
#include "binary_heap.h"
#include "fibonacci_heap.h"

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

