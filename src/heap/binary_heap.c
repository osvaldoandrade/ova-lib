#include "binary_heap.h"
#include <stdlib.h>

static void sift_up(binary_heap *h, int index);
static void sift_down(binary_heap *h, int index);

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

static void *binary_heap_pop(heap *self) {
    binary_heap *h = (binary_heap *)self->impl;
    if (h->size == 0) return NULL;

    void *top = h->data[0];
    h->data[0] = h->data[h->size - 1];
    h->size--;
    sift_down(h, 0);
    return top;
}

static void *binary_heap_peek(const heap *self) {
    binary_heap *h = (binary_heap *)self->impl;
    if (h->size == 0) return NULL;
    return h->data[0];
}

static int binary_heap_size(const heap *self) {
    binary_heap *h = (binary_heap *)self->impl;
    return h->size;
}

static void binary_heap_free(heap *self) {
    binary_heap *h = (binary_heap *) self->impl;
    free(h->data);
    free(h);
}

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

