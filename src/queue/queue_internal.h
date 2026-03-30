#ifndef QUEUE_INTERNAL_H
#define QUEUE_INTERNAL_H

#include "../../include/queue.h"

typedef struct queue_entry {
    void *data;
    struct queue_entry *next;
} queue_entry;

typedef struct queue_impl {
    queue_type type;
    queue_entry *front;
    queue_entry *rear;
    heap *p_heap;
    int length;
} queue_impl;

static inline queue_impl *queue_impl_from_queue(const queue *q) {
    return q ? (queue_impl *)q->impl : NULL;
}

#endif // QUEUE_INTERNAL_H
