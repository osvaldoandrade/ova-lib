#include <stdlib.h>
#include "../../include/queue.h"

static void linked_initialize(queue *self, comparator compare) {
    self->front = NULL;
    self->rear = NULL;
    self->length = 0;
}

static queue_entry* create_node(void *data) {
    queue_entry *new_node = malloc(sizeof(queue_entry));
    if (new_node) {
        new_node->data = data;
        new_node->next = NULL;
    }
    return new_node;
}

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

static int linked_is_empty(const queue *self) {
    return self->front == NULL;
}

static void linked_free(queue *self) {
    while (!linked_is_empty(self)) {
        linked_dequeue(self);
    }
    free(self);
}

static int linked_size(const queue *self) {
    return self->length;
}

queue *create_linked_queue() {
    queue *q = malloc(sizeof(queue));
    if (!q) return NULL;

    q->p_heap = NULL;

    q->initialize = linked_initialize;
    q->enqueue = linked_enqueue;
    q->dequeue = linked_dequeue;
    q->is_empty = linked_is_empty;
    q->free = linked_free;
    q->size = linked_size;

    q->initialize(q, NULL);
    return q;
}