/**
 * @file heap_queue.c
 * @brief Priority queue backed by a heap.
 */

#include "heap_queue.h"
#include <stdlib.h>

/**
 * @brief Create a priority queue backed by a heap.
 *
 * @param capacity Initial capacity for the underlying heap.
 * @param compare Comparison function for ordering elements.
 * @return Pointer to the created queue or NULL on failure.
 */
queue *create_heap_queue (int capacity, comparator compare) {
    queue *q = malloc(sizeof(queue));
    if (q == NULL) {
        return NULL;
    }

    /* respect the capacity requested by the caller */
    q->p_heap = create_heap(BINARY_HEAP, capacity, compare);
    q->enqueue = priority_enqueue;
    q->dequeue = priority_dequeue;
    q->is_empty = priority_is_empty;
    q->free = priority_free;
    q->size = priority_size;
    q->initialize = priority_initialize;

    q->initialize(q, compare);

    return q;
}

/**
 * @brief Initializes a priority queue.
 *
 * Initializes the internal data structure (heap) of the priority queue. This function
 * should set up the heap with the appropriate comparison function provided.
 *
 * @param self Pointer to the priority queue to be initialized.
 * @param compare The comparison function used to maintain the heap property.
 */
void priority_initialize(queue *self, comparator compare) {
    (void)compare; /* comparator already provided when heap was created */
    if (!self) {
        return;
    }

    self->front = NULL;
    self->rear = NULL;
    self->length = 0;
}

/**
 * @brief Enqueues an element in the priority queue.
 *
 * Inserts an element into the priority queue based on its priority determined by the heap's comparison function.
 * Assumes that the insertion is always successful (memory allocation issues are not handled here).
 *
 * @param self Pointer to the priority queue.
 * @param data Pointer to the data to be enqueued.
 * @return 1 on success.
 */
int priority_enqueue(queue *self, void *data) {
    if (!self || !self->p_heap) return 0;

    self->p_heap->put(self->p_heap, data);
    return 1;
}

/**
 * @brief Dequeues the highest priority element from the priority queue.
 *
 * Removes and returns the element with the highest priority from the queue, which is determined by the heap's structure.
 *
 * @param self Pointer to the priority queue.
 * @return Pointer to the data of the highest priority element, or NULL if the queue is empty.
 */
void *priority_dequeue(queue *self) {
    if (self == NULL) {
        return NULL;
    }

    if (self->p_heap == NULL) {
        return NULL;
    }

    if (self->is_empty == NULL) {
        return NULL;
    }

    if (self->p_heap->pop == NULL) {
        return NULL;
    }

    if (self->is_empty(self)) {
        return NULL;
    }

    return self->p_heap->pop(self->p_heap);
}


/**
 * @brief Checks if the priority queue is empty.
 *
 * Determines whether the priority queue is empty by checking the size of the internal heap.
 *
 * @param self Pointer to the priority queue.
 * @return 1 if the queue is empty, 0 otherwise.
 */
int priority_is_empty(const queue *self) {
    if (self == NULL) {
        return 1;
    }

    if (self->p_heap == NULL) {
        return 1;
    }

    if (self->p_heap->size == NULL) {
        return 1;
    }

    return self->p_heap->size(self->p_heap) == 0;
}


/**
 * @brief Frees all resources associated with the priority queue.
 *
 * Deallocates the heap used by the priority queue and the queue structure itself.
 *
 * @param self Pointer to the priority queue to be freed.
 */
void priority_free(queue *self) {
    if (!self) return;
    if (self->p_heap) {
        self->p_heap->free(self->p_heap);
    }
    free(self);
}

/**
 * @brief Returns the number of elements in the priority queue.
 *
 * Retrieves the number of elements currently in the priority queue, which corresponds to the size of the internal heap.
 *
 * @param self Pointer to the priority queue.
 * @return The number of elements in the priority queue.
 */
int priority_size(const queue *self) {
    if (self == NULL || self->p_heap == NULL) {
        return 0;
    }
    return self->p_heap->size(self->p_heap);
}

