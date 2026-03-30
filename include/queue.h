#ifndef QUEUE_H
#define QUEUE_H

#include "heap.h"
#include "types.h"

typedef enum {
    QUEUE_TYPE_NORMAL,
    QUEUE_TYPE_PRIORITY
} queue_type;

/**
 * @brief Public queue object.
 *
 * Concrete storage details live in @p impl.
 */
typedef struct queue {
    void *impl;

    /**
     * @brief Enqueue an item.
     *
     * @param self Queue instance.
     * @param data Payload pointer to enqueue.
     * @return 1 on success, 0 on failure.
     */
    int (*enqueue)(struct queue *self, void *data);

    /**
     * @brief Dequeue and return the next item.
     *
     * @param self Queue instance.
     * @return Payload pointer, or NULL when empty.
     */
    void *(*dequeue)(struct queue *self);

    /**
     * @brief Check whether the queue is empty.
     *
     * @param self Queue instance.
     * @return 1 when empty, 0 otherwise.
     */
    int (*is_empty)(const struct queue *self);

    /**
     * @brief Return the current element count.
     *
     * @param self Queue instance.
     * @return Number of stored elements.
     */
    int (*size)(const struct queue *self);

    /**
     * @brief Release the queue and its internal allocations.
     *
     * The queue does not free user payloads.
     *
     * @param self Queue instance.
     */
    void (*free)(struct queue *self);
} queue;

/**
 * @brief Create a new queue.
 *
 * @param type Queue backend to construct.
 * @param capacity Capacity hint for priority queues.
 * @param compare Comparator used by priority queues.
 * @return New queue instance, or NULL on failure.
 */
queue *create_queue(queue_type type, int capacity, comparator compare);

#endif // QUEUE_H
