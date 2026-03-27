#ifndef CAPACITY_UTILS_H
#define CAPACITY_UTILS_H

#include <limits.h>

/**
 * @brief Safely doubles the given capacity without integer overflow.
 *
 * If doubling would exceed INT_MAX, INT_MAX is returned instead so that
 * callers can allocate the largest safe buffer and avoid undefined behaviour.
 *
 * @param current_capacity The current capacity value to double.
 * @return The doubled capacity, or INT_MAX if doubling would overflow.
 */
static inline int safe_double_capacity(int current_capacity) {
    if (current_capacity > INT_MAX / 2) {
        return INT_MAX;
    }
    return current_capacity * 2;
}

#endif /* CAPACITY_UTILS_H */
