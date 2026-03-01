#ifndef DEQUE_H
#define DEQUE_H

#include "types.h"

/**
 * @file deque.h
 * @brief Double-ended queue (deque) data structure with efficient operations at both ends.
 *
 * A deque is a linear data structure that allows insertion and removal of elements
 * from both the front and the back in O(1) amortized time. It is implemented using
 * a circular buffer with dynamic resizing.
 *
 * The implementation provides:
 * - O(1) amortized push and pop operations at both ends
 * - O(1) random access via indexing
 * - Geometric growth (2x capacity) when resizing
 * - Automatic capacity management
 *
 * Memory Management:
 * - The deque stores pointers to elements but does not own the data
 * - Callers are responsible for managing the lifetime of stored elements
 * - Call deque_free() to release the deque structure itself
 *
 * Example Usage:
 * @code
 *   deque *dq = create_deque(10);
 *   int value = 42;
 *   deque_push_back(dq, &value);
 *   int *result = deque_pop_front(dq);
 *   deque_free(dq);
 * @endcode
 */

typedef struct deque deque;

/**
 * @brief Creates a new deque with the specified initial capacity.
 *
 * Allocates and initializes a deque data structure with the given capacity.
 * If capacity is zero or negative, a default capacity of 16 is used.
 *
 * @param capacity Initial capacity for the deque (must be non-negative).
 * @return A pointer to the newly created deque, or NULL if allocation fails.
 */
deque* create_deque(int capacity);

/**
 * @brief Inserts an element at the front of the deque.
 *
 * Adds the given element to the front (head) of the deque. If the deque is
 * at full capacity, it will automatically resize (double capacity) before
 * insertion. This operation has O(1) amortized time complexity.
 *
 * @param d The deque to insert into.
 * @param element Pointer to the element to insert.
 */
void deque_push_front(deque *d, void *element);

/**
 * @brief Inserts an element at the back of the deque.
 *
 * Adds the given element to the back (tail) of the deque. If the deque is
 * at full capacity, it will automatically resize (double capacity) before
 * insertion. This operation has O(1) amortized time complexity.
 *
 * @param d The deque to insert into.
 * @param element Pointer to the element to insert.
 */
void deque_push_back(deque *d, void *element);

/**
 * @brief Removes and returns the element at the front of the deque.
 *
 * Removes the element from the front (head) of the deque and returns a
 * pointer to it. The deque does not free the element; the caller is
 * responsible for managing its lifetime.
 *
 * @param d The deque to remove from.
 * @return Pointer to the removed element, or NULL if the deque is empty.
 */
void* deque_pop_front(deque *d);

/**
 * @brief Removes and returns the element at the back of the deque.
 *
 * Removes the element from the back (tail) of the deque and returns a
 * pointer to it. The deque does not free the element; the caller is
 * responsible for managing its lifetime.
 *
 * @param d The deque to remove from.
 * @return Pointer to the removed element, or NULL if the deque is empty.
 */
void* deque_pop_back(deque *d);

/**
 * @brief Returns the element at the front without removing it.
 *
 * Peeks at the element at the front (head) of the deque without modifying
 * the deque structure.
 *
 * @param d The deque to peek into.
 * @return Pointer to the front element, or NULL if the deque is empty.
 */
void* deque_peek_front(deque *d);

/**
 * @brief Returns the element at the back without removing it.
 *
 * Peeks at the element at the back (tail) of the deque without modifying
 * the deque structure.
 *
 * @param d The deque to peek into.
 * @return Pointer to the back element, or NULL if the deque is empty.
 */
void* deque_peek_back(deque *d);

/**
 * @brief Accesses an element at the specified index.
 *
 * Returns the element at the given index in the deque. Index 0 refers to
 * the front element, and index (size - 1) refers to the back element.
 * This operation has O(1) time complexity.
 *
 * @param d The deque to access.
 * @param index The zero-based index of the element (0 <= index < size).
 * @return Pointer to the element at the specified index, or NULL if the
 *         index is out of bounds or the deque is NULL.
 */
void* deque_get(deque *d, int index);

/**
 * @brief Returns the number of elements in the deque.
 *
 * @param d The deque to query.
 * @return The number of elements currently stored in the deque, or 0 if
 *         the deque is NULL.
 */
int deque_size(deque *d);

/**
 * @brief Checks if the deque is empty.
 *
 * @param d The deque to check.
 * @return true if the deque is empty or NULL, false otherwise.
 */
bool deque_is_empty(deque *d);

/**
 * @brief Frees the memory used by the deque.
 *
 * Releases all memory associated with the deque structure, including the
 * internal buffer. This function does not free the elements themselves;
 * the caller must manage element lifetime separately.
 *
 * @param d The deque to free.
 */
void deque_free(deque *d);

#endif // DEQUE_H
