#include "../../include/deque.h"
#include "../utils/capacity_utils.h"
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAPACITY 16
#define MIN_CAPACITY 4

struct deque {
    void **buffer;      // Circular buffer for storing elements
    int capacity;       // Total capacity of the buffer
    int size;           // Current number of elements
    int front;          // Index of the front element
};

/**
 * @brief Creates a new deque with the specified initial capacity.
 */
deque* create_deque(int capacity) {
    deque *d = (deque*)malloc(sizeof(deque));
    if (!d) {
        return NULL;
    }

    // Use default capacity if requested capacity is invalid
    if (capacity <= 0) {
        capacity = DEFAULT_CAPACITY;
    }

    d->buffer = (void**)calloc(capacity, sizeof(void*));
    if (!d->buffer) {
        free(d);
        return NULL;
    }

    d->capacity = capacity;
    d->size = 0;
    d->front = 0;

    return d;
}

/**
 * @brief Helper function to resize the deque when it's full.
 * 
 * Doubles the capacity and reorganizes elements to maintain logical order.
 */
static int deque_resize(deque *d) {
    if (!d) {
        return -1;
    }

    int new_capacity = safe_double_capacity(d->capacity);
    if (new_capacity == d->capacity) {
        return -1; // Already at maximum capacity
    }
    void **new_buffer = (void**)calloc(new_capacity, sizeof(void*));
    if (!new_buffer) {
        return -1;
    }

    // Copy elements in logical order starting from front
    for (int i = 0; i < d->size; i++) {
        int physical_index = (d->front + i) % d->capacity;
        new_buffer[i] = d->buffer[physical_index];
    }

    free(d->buffer);
    d->buffer = new_buffer;
    d->capacity = new_capacity;
    d->front = 0;

    return 0;
}

/**
 * @brief Inserts an element at the front of the deque.
 */
void deque_push_front(deque *d, void *element) {
    if (!d) {
        return;
    }

    // Resize if full
    if (d->size == d->capacity) {
        if (deque_resize(d) != 0) {
            return;
        }
    }

    // Move front backwards (circular)
    d->front = (d->front - 1 + d->capacity) % d->capacity;
    d->buffer[d->front] = element;
    d->size++;
}

/**
 * @brief Inserts an element at the back of the deque.
 */
void deque_push_back(deque *d, void *element) {
    if (!d) {
        return;
    }

    // Resize if full
    if (d->size == d->capacity) {
        if (deque_resize(d) != 0) {
            return;
        }
    }

    // Calculate back position (circular)
    int back = (d->front + d->size) % d->capacity;
    d->buffer[back] = element;
    d->size++;
}

/**
 * @brief Removes and returns the element at the front of the deque.
 */
void* deque_pop_front(deque *d) {
    if (!d || d->size == 0) {
        return NULL;
    }

    void *element = d->buffer[d->front];
    d->buffer[d->front] = NULL;
    d->front = (d->front + 1) % d->capacity;
    d->size--;

    return element;
}

/**
 * @brief Removes and returns the element at the back of the deque.
 */
void* deque_pop_back(deque *d) {
    if (!d || d->size == 0) {
        return NULL;
    }

    // Calculate back position
    int back = (d->front + d->size - 1) % d->capacity;
    void *element = d->buffer[back];
    d->buffer[back] = NULL;
    d->size--;

    return element;
}

/**
 * @brief Returns the element at the front without removing it.
 */
void* deque_peek_front(deque *d) {
    if (!d || d->size == 0) {
        return NULL;
    }

    return d->buffer[d->front];
}

/**
 * @brief Returns the element at the back without removing it.
 */
void* deque_peek_back(deque *d) {
    if (!d || d->size == 0) {
        return NULL;
    }

    int back = (d->front + d->size - 1) % d->capacity;
    return d->buffer[back];
}

/**
 * @brief Accesses an element at the specified index.
 */
void* deque_get(deque *d, int index) {
    if (!d || index < 0 || index >= d->size) {
        return NULL;
    }

    int physical_index = (d->front + index) % d->capacity;
    return d->buffer[physical_index];
}

/**
 * @brief Returns the number of elements in the deque.
 */
int deque_size(deque *d) {
    if (!d) {
        return 0;
    }

    return d->size;
}

/**
 * @brief Checks if the deque is empty.
 */
bool deque_is_empty(deque *d) {
    if (!d) {
        return true;
    }

    return d->size == 0;
}

/**
 * @brief Frees the memory used by the deque.
 */
void deque_free(deque *d) {
    if (!d) {
        return;
    }

    if (d->buffer) {
        free(d->buffer);
    }

    free(d);
}
