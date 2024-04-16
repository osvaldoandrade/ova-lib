#ifndef HEAP_QUEUE_H
#define HEAP_QUEUE_H

#include "../../include/queue.h"
#include "../../include/heap.h"

void priority_initialize(queue *self, comparator compare);
int priority_enqueue(queue *self, void *data);
void *priority_dequeue(queue *self);
int priority_is_empty(const queue *self);
void priority_free(queue *self);
int priority_size(const queue *self);

queue *create_heap_queue (int capacity, comparator compare);

#endif // HEAP_QUEUE_H
