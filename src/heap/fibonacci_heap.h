#ifndef FIBONACCI_HEAP_H
#define FIBONACCI_HEAP_H

#include "../../include/heap.h"


typedef struct fib_node {
    void *data;
    int degree;
    struct fib_node *parent;
    struct fib_node *child;
    struct fib_node *left;
    struct fib_node *right;
    char mark;
} fib_node;

typedef struct fibonacci_heap {
    fib_node *min;
    int n;  // number of nodes
    comparator cmp;
} fibonacci_heap;

heap *create_fibonacci_heap(comparator compare_function);

#endif // FIBONACCI_HEAP_H
