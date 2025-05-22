/**
 * @file queue.c
 * @brief Queue creation helpers.
 */

#include <stdlib.h>
#include "heap_queue.h"
#include "linked_queue.h"

/**
 * @brief Factory method that creates either a normal or priority queue.
 */
queue *create_queue(queue_type type, int capacity, comparator compare) {
    switch (type) {
        case QUEUE_TYPE_NORMAL:
            return create_linked_queue();
        case QUEUE_TYPE_PRIORITY:
            return create_heap_queue(capacity, compare);
        default:
            return NULL;
    }
}