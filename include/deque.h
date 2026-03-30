#ifndef DEQUE_H
#define DEQUE_H

#include "types.h"

/**
 * @brief Public deque object.
 *
 * The deque stores user-owned payload pointers. Concrete storage details live
 * in @p impl.
 */
typedef struct deque {
    void *impl;

    /**
     * @brief Push an item at the front.
     *
     * @param self Deque instance.
     * @param element Payload pointer to insert.
     */
    void (*push_front)(struct deque *self, void *element);

    /**
     * @brief Push an item at the back.
     *
     * @param self Deque instance.
     * @param element Payload pointer to insert.
     */
    void (*push_back)(struct deque *self, void *element);

    /**
     * @brief Pop and return the front item.
     *
     * @param self Deque instance.
     * @return Front payload pointer, or NULL when empty.
     */
    void *(*pop_front)(struct deque *self);

    /**
     * @brief Pop and return the back item.
     *
     * @param self Deque instance.
     * @return Back payload pointer, or NULL when empty.
     */
    void *(*pop_back)(struct deque *self);

    /**
     * @brief Return the front item without removing it.
     *
     * @param self Deque instance.
     * @return Front payload pointer, or NULL when empty.
     */
    void *(*peek_front)(const struct deque *self);

    /**
     * @brief Return the back item without removing it.
     *
     * @param self Deque instance.
     * @return Back payload pointer, or NULL when empty.
     */
    void *(*peek_back)(const struct deque *self);

    /**
     * @brief Return the item at the given logical index.
     *
     * @param self Deque instance.
     * @param index Zero-based logical index.
     * @return Payload pointer, or NULL when out of bounds.
     */
    void *(*get)(const struct deque *self, int index);

    /**
     * @brief Return the current element count.
     *
     * @param self Deque instance.
     * @return Number of stored elements.
     */
    int (*size)(const struct deque *self);

    /**
     * @brief Check whether the deque is empty.
     *
     * @param self Deque instance.
     * @return true when empty, false otherwise.
     */
    bool (*is_empty)(const struct deque *self);

    /**
     * @brief Release the deque and its internal allocations.
     *
     * The deque does not free user payloads.
     *
     * @param self Deque instance.
     */
    void (*free)(struct deque *self);
} deque;

/**
 * @brief Create a new deque.
 *
 * @param capacity Initial capacity hint. Non-positive values use a default.
 * @return New deque instance, or NULL on failure.
 */
deque *create_deque(int capacity);

#endif // DEQUE_H
