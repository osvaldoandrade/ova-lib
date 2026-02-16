#include "../../include/bloom_filter.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct bloom_filter {
    size_t m_bits;
    size_t k_hashes;
    uint8_t *bits;

    size_t expected_elements;
    double target_fpp;

    size_t items_added;
};

static uint64_t fnv1a64_seeded(const void *data, size_t len, uint64_t seed) {
    const uint8_t *bytes = (const uint8_t *)data;
    uint64_t hash = 14695981039346656037ULL ^ seed;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint64_t)bytes[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

static void bloom_set_bit(uint8_t *bits, size_t idx) {
    bits[idx >> 3u] |= (uint8_t)(1u << (idx & 7u));
}

static int bloom_get_bit(const uint8_t *bits, size_t idx) {
    return (bits[idx >> 3u] & (uint8_t)(1u << (idx & 7u))) != 0;
}

static size_t bloom_index(uint64_t h1, uint64_t h2, size_t i, size_t m_bits) {
    uint64_t x = h1 + (uint64_t)i * h2;
    return (size_t)(x % (uint64_t)m_bits);
}

static int bloom_compute_params(int expected_elements,
                                double false_positive_rate,
                                size_t *out_m_bits,
                                size_t *out_k_hashes) {
    if (!out_m_bits || !out_k_hashes) {
        return 0;
    }
    *out_m_bits = 0;
    *out_k_hashes = 0;

    if (expected_elements <= 0) {
        return 0;
    }
    if (!(false_positive_rate > 0.0 && false_positive_rate < 1.0)) {
        return 0;
    }

    const double n = (double)expected_elements;
    const double p = false_positive_rate;
    const double ln2 = log(2.0);
    const double ln2_sq = ln2 * ln2;

    double m = -n * log(p) / ln2_sq;
    if (!(m > 1.0)) {
        m = 1.0;
    }

    double k = (m / n) * ln2;
    if (!(k > 1.0)) {
        k = 1.0;
    }

    size_t m_bits = (size_t)ceil(m);
    size_t k_hashes = (size_t)llround(k);
    if (k_hashes == 0) {
        k_hashes = 1;
    }

    /* Guard against overflow in byte allocation (m_bits -> bytes). */
    size_t bytes = (m_bits + 7u) / 8u;
    if (bytes == 0 || bytes > (SIZE_MAX / sizeof(uint8_t))) {
        return 0;
    }

    *out_m_bits = m_bits;
    *out_k_hashes = k_hashes;
    return 1;
}

bloom_filter *create_bloom_filter(int expected_elements, double false_positive_rate) {
    size_t m_bits = 0;
    size_t k_hashes = 0;
    if (!bloom_compute_params(expected_elements, false_positive_rate, &m_bits, &k_hashes)) {
        return NULL;
    }

    bloom_filter *bf = (bloom_filter *)calloc(1, sizeof(bloom_filter));
    if (!bf) {
        return NULL;
    }

    size_t bytes = (m_bits + 7u) / 8u;
    bf->bits = (uint8_t *)calloc(bytes, 1);
    if (!bf->bits) {
        free(bf);
        return NULL;
    }

    bf->m_bits = m_bits;
    bf->k_hashes = k_hashes;
    bf->expected_elements = (size_t)expected_elements;
    bf->target_fpp = false_positive_rate;
    bf->items_added = 0;

    return bf;
}

void bloom_filter_free(bloom_filter *bf) {
    if (!bf) {
        return;
    }
    free(bf->bits);
    bf->bits = NULL;
    free(bf);
}

void bloom_filter_add(bloom_filter *bf, const void *element, size_t len) {
    if (!bf || !bf->bits || !element || len == 0 || bf->m_bits == 0 || bf->k_hashes == 0) {
        return;
    }

    uint64_t h1 = fnv1a64_seeded(element, len, 0x243f6a8885a308d3ULL);
    uint64_t h2 = fnv1a64_seeded(element, len, 0x9e3779b97f4a7c15ULL);
    if (h2 == 0) {
        h2 = 0x27d4eb2f165667c5ULL;
    }

    for (size_t i = 0; i < bf->k_hashes; i++) {
        size_t idx = bloom_index(h1, h2, i, bf->m_bits);
        bloom_set_bit(bf->bits, idx);
    }

    bf->items_added++;
}

bool bloom_filter_might_contain(const bloom_filter *bf, const void *element, size_t len) {
    if (!bf || !bf->bits || !element || len == 0 || bf->m_bits == 0 || bf->k_hashes == 0) {
        return false;
    }

    uint64_t h1 = fnv1a64_seeded(element, len, 0x243f6a8885a308d3ULL);
    uint64_t h2 = fnv1a64_seeded(element, len, 0x9e3779b97f4a7c15ULL);
    if (h2 == 0) {
        h2 = 0x27d4eb2f165667c5ULL;
    }

    for (size_t i = 0; i < bf->k_hashes; i++) {
        size_t idx = bloom_index(h1, h2, i, bf->m_bits);
        if (!bloom_get_bit(bf->bits, idx)) {
            return false;
        }
    }
    return true;
}

void bloom_filter_clear(bloom_filter *bf) {
    if (!bf || !bf->bits || bf->m_bits == 0) {
        return;
    }

    size_t bytes = (bf->m_bits + 7u) / 8u;
    memset(bf->bits, 0, bytes);
    bf->items_added = 0;
}

double bloom_filter_current_fpp(const bloom_filter *bf) {
    if (!bf || bf->m_bits == 0 || bf->k_hashes == 0) {
        return 0.0;
    }
    if (bf->items_added == 0) {
        return 0.0;
    }

    const double m = (double)bf->m_bits;
    const double k = (double)bf->k_hashes;
    const double n = (double)bf->items_added;

    double x = -k * n / m;
    double p = pow(1.0 - exp(x), k);

    if (p < 0.0) {
        return 0.0;
    }
    if (p > 1.0) {
        return 1.0;
    }
    return p;
}

