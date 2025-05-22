/**
 * @file binary_heap.c
 * @brief Binary heap implementation.
 */

#include "binary_heap.h"
#include <stdlib.h>

static void sift_up(binary_heap *h, int index);
static void sift_down(binary_heap *h, int index);

/**
 * @brief Initialize a binary heap structure.
 *
 * Allocates memory and sets up the internal array with the given capacity.
 *
 * @param initial_capacity Starting capacity of the heap.
 * @param cmp Comparator used for ordering elements.
 * @return Pointer to the initialized structure or NULL on failure.
 */
binary_heap *binary_heap_init(int initial_capacity, comparator cmp) {
    binary_heap *h = malloc(sizeof(binary_heap));
    if (!h) return NULL;

    h->data = malloc(initial_capacity * sizeof(void *));
    if (!h->data) {
        free(h);
        return NULL;
    }

    h->capacity = initial_capacity;
    h->size = 0;
    h->cmp = cmp;
    return h;
}

/**
 * @brief Insert an element into the binary heap.
 *
 * @param self Heap wrapper used for insertion.
 * @param item Element to insert.
 */
static void binary_heap_put(heap *self, void *item) {
    binary_heap *h = (binary_heap *)self->impl;
    if (h->size == h->capacity) {
        h->capacity *= 2;
        h->data = realloc(h->data, h->capacity * sizeof(void *));
    }
    h->data[h->size] = item;
    sift_up(h, h->size);
    h->size++;
}

/**
 * @brief Remove and return the top element from the binary heap.
 *
 * @param self Heap wrapper whose top element will be removed.
 * @return Pointer to the removed element or NULL if the heap is empty.
 */
static void *binary_heap_pop(heap *self) {
    binary_heap *h = (binary_heap *)self->impl;
    if (h->size == 0) return NULL;

    void *top = h->data[0];
    h->data[0] = h->data[h->size - 1];
    h->size--;
    sift_down(h, 0);
    return top;
}

/**
 * @brief Get the top element of the binary heap without removing it.
 *
 * @param self Heap wrapper to peek into.
 * @return Top element or NULL if the heap is empty.
 */
static void *binary_heap_peek(const heap *self) {
    binary_heap *h = (binary_heap *)self->impl;
    if (h->size == 0) return NULL;
    return h->data[0];
}

/**
 * @brief Return the number of elements stored in the heap.
 *
 * @param self Heap wrapper to query.
 * @return Current heap size.
 */
static int binary_heap_size(const heap *self) {
    binary_heap *h = (binary_heap *)self->impl;
    return h->size;
}

/**
 * @brief Release resources held by a binary heap.
 *
 * @param self Heap wrapper to destroy.
 */
static void binary_heap_free(heap *self) {
    binary_heap *h = (binary_heap *) self->impl;
    free(h->data);
    free(h);
}

/**
 * @brief Create a generic heap backed by a binary heap implementation.
 *
 * @param initial_capacity Desired capacity of the heap.
 * @param compare_function Comparator for ordering elements.
 * @return Pointer to a heap instance or NULL on failure.
 */
heap *create_binary_heap(int initial_capacity, comparator compare_function) {
    binary_heap *bh = binary_heap_init(initial_capacity, compare_function);
    if (!bh) return NULL;

    heap *h = malloc(sizeof(heap));
    if (!h) {
        free(bh);
        return NULL;
    }

    h->impl = bh;
    h->put = binary_heap_put;
    h->pop = binary_heap_pop;
    h->peek = binary_heap_peek;
    h->size = binary_heap_size;
    h->free = binary_heap_free;
    return h;
}

/**
 * @brief Move an element up the tree to restore heap property.
 */
static void sift_up(binary_heap *h, int index) {
    void *item = h->data[index];
    while (index > 0) {
        int parent_idx = (index - 1) / 2;
        if (h->cmp(h->data[parent_idx], item) >= 0) break;
        h->data[index] = h->data[parent_idx];
        index = parent_idx;
    }
    h->data[index] = item;
}

/**
 * @brief Move an element down the tree to restore heap property.
 */
static void sift_down(binary_heap *h, int index) {
    int left, right, largest;
    void *temp;
    while (index < h->size / 2) {
        left = 2 * index + 1;
        right = 2 * index + 2;
        largest = index;

        if (left < h->size && h->cmp(h->data[left], h->data[largest]) > 0) {
            largest = left;
        }
        if (right < h->size && h->cmp(h->data[right], h->data[largest]) > 0) {
            largest = right;
        }
        if (largest == index) break;

        temp = h->data[index];
        h->data[index] = h->data[largest];
        h->data[largest] = temp;
        index = largest;
    }
}

