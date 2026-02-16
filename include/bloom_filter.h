#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include "types.h"

typedef struct bloom_filter bloom_filter;

/**
 * @brief Create a Bloom filter sized for @p expected_elements and @p false_positive_rate.
 *
 * @param expected_elements Expected number of inserted elements (n).
 * @param false_positive_rate Target false positive probability (p), must be 0 < p < 1.
 * @return A new bloom_filter instance or NULL on invalid parameters / allocation failure.
 */
bloom_filter *create_bloom_filter(int expected_elements, double false_positive_rate);

void bloom_filter_free(bloom_filter *bf);

/**
 * @brief Add an element to the Bloom filter.
 *
 * The element is treated as an opaque byte sequence of length @p len.
 */
void bloom_filter_add(bloom_filter *bf, const void *element, size_t len);

/**
 * @brief Check membership.
 *
 * Returns true if the element might be present (possible false positives),
 * and false if the element is definitely not present (no false negatives).
 */
bool bloom_filter_might_contain(const bloom_filter *bf, const void *element, size_t len);

/**
 * @brief Clear all bits and reset insertion counters.
 */
void bloom_filter_clear(bloom_filter *bf);

/**
 * @brief Estimate the current false positive probability.
 *
 * The estimate uses the standard Bloom filter approximation:
 *   p ~= (1 - exp(-(k*n)/m))^k
 * where n is the number of insert calls performed on this instance.
 */
double bloom_filter_current_fpp(const bloom_filter *bf);

#endif // BLOOM_FILTER_H

