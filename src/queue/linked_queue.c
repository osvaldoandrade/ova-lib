/**
 * @file linked_queue.c
 * @brief Linked list based queue implementation.
 */

#include <stdlib.h>
#include "../../include/queue.h"

/**
 * @brief Initialize an empty linked queue.
 */
static void linked_initialize(queue *self, comparator compare) {
    self->front = NULL;
    self->rear = NULL;
    self->length = 0;
}

/**
 * @brief Allocate a new queue node.
 */
static queue_entry* create_node(void *data) {
    queue_entry *new_node = malloc(sizeof(queue_entry));
    if (new_node) {
        new_node->data = data;
        new_node->next = NULL;
    }
    return new_node;
}

/**
 * @brief Enqueue data at the end of the linked queue.
 */
static int linked_enqueue(queue *self, void *data) {
    queue_entry *new_node = create_node(data);
    if (!new_node) return 0;

    if (self->rear) {
        self->rear->next = new_node;
    }
    self->rear = new_node;

    if (!self->front) {
        self->front = new_node;
    }

    self->length++;
    return 1;
}

/**
 * @brief Remove and return the element at the front of the queue.
 */
static void* linked_dequeue(queue *self) {
    if (!self->front) return NULL;

    queue_entry *temp = self->front;
    void *data = temp->data;
    self->front = self->front->next;

    if (!self->front) {
        self->rear = NULL;
    }

    free(temp);
    self->length--;
    return data;
}

/**
 * @brief Check whether the queue is empty.
 */
static int linked_is_empty(const queue *self) {
    return self->front == NULL;
}

/**
 * @brief Free all nodes and the queue itself.
 */
static void linked_free(queue *self) {
    while (!linked_is_empty(self)) {
        linked_dequeue(self);
    }
    free(self);
}

/**
 * @brief Return the number of elements currently in the queue.
 */
static int linked_size(const queue *self) {
    return self->length;
}

/**
 * @brief Create a queue implemented with a linked list.
 */
queue *create_linked_queue() {
    queue *q = malloc(sizeof(queue));
    if (!q) return NULL;

    q->initialize = linked_initialize;
    q->enqueue = linked_enqueue;
    q->dequeue = linked_dequeue;
    q->is_empty = linked_is_empty;
    q->free = linked_free;
    q->size = linked_size;

    q->initialize(q, NULL);
    return q;
}