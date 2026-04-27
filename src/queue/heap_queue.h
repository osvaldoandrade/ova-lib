#ifndef HEAP_QUEUE_H
#define HEAP_QUEUE_H

#include "queue_internal.h"

ova_error_code priority_enqueue(queue *self, void *data);
void *priority_dequeue(queue *self);
int priority_is_empty(const queue *self);
void priority_free(queue *self);
int priority_size(const queue *self);

queue *create_heap_queue (int capacity, comparator compare);

#endif // HEAP_QUEUE_H
