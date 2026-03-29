#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

/**
 * @file bloom_filter.h
 * @brief Probabilistic data structure for approximate set membership testing.
 *
 * A Bloom filter uses multiple hash functions to compactly represent a set.
 * It may produce false positives but never false negatives.
 */

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

/**
 * @brief Frees all memory associated with the Bloom filter.
 *
 * @param bf The Bloom filter to free.
 */
void bloom_filter_free(bloom_filter *bf);

/**
 * @brief Add an element to the Bloom filter.
 *
 * The element is treated as an opaque byte sequence of length @p len.
 *
 * @param bf The Bloom filter.
 * @param element Pointer to the element data.
 * @param len Length of the element in bytes.
 */
void bloom_filter_add(bloom_filter *bf, const void *element, size_t len);

/**
 * @brief Check membership.
 *
 * Returns true if the element might be present (possible false positives),
 * and false if the element is definitely not present (no false negatives).
 *
 * @param bf The Bloom filter.
 * @param element Pointer to the element data.
 * @param len Length of the element in bytes.
 * @return true if the element might be present, false if definitely absent.
 */
bool bloom_filter_might_contain(const bloom_filter *bf, const void *element, size_t len);

/**
 * @brief Clear all bits and reset insertion counters.
 *
 * @param bf The Bloom filter to clear.
 */
void bloom_filter_clear(bloom_filter *bf);

/**
 * @brief Estimate the current false positive probability.
 *
 * The estimate uses the standard Bloom filter approximation:
 *   p ~= (1 - exp(-(k*n)/m))^k
 * where n is the number of insert calls performed on this instance.
 *
 * @param bf The Bloom filter.
 * @return The estimated false positive probability.
 */
double bloom_filter_current_fpp(const bloom_filter *bf);

#endif // BLOOM_FILTER_H

