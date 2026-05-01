#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include "types.h"

/**
 * @brief Public Bloom filter object.
 *
 * Concrete storage details live in @p impl.
 */
typedef struct bloom_filter {
    void *impl;
    void *user_data; /**< User-provided context pointer. */

    /**
     * @brief Add an element to the filter.
     *
     * The payload is treated as an opaque byte sequence of length @p len.
     *
     * @param self Bloom filter instance.
     * @param element Byte sequence pointer.
     * @param len Number of bytes to hash.
     */
    void (*add)(struct bloom_filter *self, const void *element, size_t len);

    /**
     * @brief Check whether an element might be present.
     *
     * @param self Bloom filter instance.
     * @param element Byte sequence pointer.
     * @param len Number of bytes to hash.
     * @return true for possible membership, false for definite absence.
     */
    bool (*might_contain)(const struct bloom_filter *self, const void *element, size_t len);

    /**
     * @brief Reset all bits and insertion counters.
     *
     * @param self Bloom filter instance.
     */
    void (*clear)(struct bloom_filter *self);

    /**
     * @brief Estimate the current false-positive probability.
     *
     * @param self Bloom filter instance.
     * @return Estimated false-positive rate in the range [0, 1].
     */
    double (*current_fpp)(const struct bloom_filter *self);

    /**
     * @brief Release the filter and its internal allocations.
     *
     * @param self Bloom filter instance.
     */
    void (*free)(struct bloom_filter *self);
} bloom_filter;

/**
 * @brief Create a Bloom filter sized for the given workload.
 *
 * @param expected_elements Expected number of inserted elements.
 * @param false_positive_rate Target false-positive probability in the range (0, 1).
 * @return New Bloom filter instance, or NULL on invalid parameters / allocation failure.
 */
bloom_filter *create_bloom_filter(int expected_elements, double false_positive_rate);

#endif // BLOOM_FILTER_H
