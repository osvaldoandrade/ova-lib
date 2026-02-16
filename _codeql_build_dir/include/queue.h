#ifndef QUEUE_H
#define QUEUE_H

#include "types.h"
#include "heap.h"

typedef struct queue_entry queue_entry;
typedef struct queue queue;

struct queue_entry {
    void *data;
    queue_entry *next;
};

typedef enum {
    QUEUE_TYPE_NORMAL, QUEUE_TYPE_PRIORITY
} queue_type;

/**
 * @file queue.h
 * @brief This file contains the definition of a Queue data structure.
 */
struct queue {
    queue_entry *front;
    queue_entry *rear;
    heap *p_heap;
    int length;

    /**
     * @brief Initializes the queue with a given comparator.
     *
     * This function initializes the queue by setting its front and rear
     * pointers to NULL, and assigns the given comparator function to the
     * queue's `compare` member variable.
     *
     * @param self The queue to be initialized.
     * @param compare The comparator function to be assigned.
     */
    void (*initialize)(queue *self, comparator compare);

    /**
     * @brief Enqueues a data element into the queue.
     *
     * This function enqueues the given data element into the queue.
     *
     * @param self The queue to enqueue the data into.
     * @param data The data element to enqueue.
     * @return An integer representing the status of the enqueue operation.
     *         - 0 if the enqueue was successful.
     *         - 1 if the enqueue failed due to an error.
     */
    int (*enqueue)(queue *self, void *data);


    /**
     * @brief Dequeues an element from the queue.
     *
     * This function dequeues an element from the queue by removing the element at the front of the queue.
     *
     * @param self The queue from which to dequeue the element.
     * @return A pointer to the dequeued element.
     */
    void *(*dequeue)(queue *self);

    /**
     * @brief Checks if the given queue is empty.
     *
     * This function checks if the given queue is empty.
     *
     * @param self The queue to check.
     * @return An integer representing whether the queue is empty or not.
     *         - 1 if the queue is empty.
     *         - 0 if the queue is not empty.
     */
    int (*is_empty)(const queue *self);

    /**
     * @brief Returns the size of the queue.
     *
     * This function returns the number of elements currently in the queue.
     *
     * @param self The queue to get the size of.
     * @return The size of the queue.
     */
    int (*size)(const queue *self);

    /**
     * @fn void (*)(queue *) free
     * @brief Frees the memory used by the given queue.
     *
     * This function frees the memory used by the given queue, including all the elements in the queue.
     *
     * @param self The queue to be freed.
     */
    void (*free)(queue *self);
};

/**
 * @brief Creates a new queue of the specified type.
 *
 * This function creates a new queue based on the specified type.
 * If the type is QUEUE_TYPE_NORMAL, a linked list based queue will be created.
 * If the type is QUEUE_TYPE_PRIORITY, a heap based queue will be created with the specified capacity and comparator function.
 *
 * @param type The type of the queue to create.
 * @param capacity The capacity of the queue (for priority queues only).
 * @param compare The comparator function used for priority queues.
 * @return A pointer to the newly created queue, or NULL if the type is not valid.
 */
queue *create_queue(queue_type type, int capacity, comparator compare);

#endif // QUEUE_H