#include "../../include/bloom_filter.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct bloom_filter_impl {
    size_t m_bits;
    size_t k_hashes;
    uint8_t *bits;
    size_t expected_elements;
    double target_fpp;
    size_t items_added;
} bloom_filter_impl;

static bloom_filter_impl *bloom_filter_impl_from_self(const bloom_filter *self) {
    return self ? (bloom_filter_impl *)self->impl : NULL;
}

#define FNV1A64_OFFSET 14695981039346656037ULL
#define FNV1A64_PRIME  1099511628211ULL
#define BLOOM_SEED_1   0x243f6a8885a308d3ULL
#define BLOOM_SEED_2   0x9e3779b97f4a7c15ULL
#define BLOOM_H2_FALLBACK 0x27d4eb2f165667c5ULL

/* Compute two seeded FNV-1a 64-bit hashes of the same buffer in a single
 * pass over the data. Matches the per-seed result of the previous helper
 * exactly (verified by table-driven tests) but loads the input only once,
 * which is the bottleneck on warm bloom filters with non-trivial keys. */
static void fnv1a64_dual(const void *data, size_t len,
                         uint64_t *h1_out, uint64_t *h2_out) {
    const uint8_t *bytes = (const uint8_t *)data;
    uint64_t h1 = FNV1A64_OFFSET ^ BLOOM_SEED_1;
    uint64_t h2 = FNV1A64_OFFSET ^ BLOOM_SEED_2;
    for (size_t i = 0; i < len; i++) {
        const uint64_t b = (uint64_t)bytes[i];
        h1 = (h1 ^ b) * FNV1A64_PRIME;
        h2 = (h2 ^ b) * FNV1A64_PRIME;
    }
    *h1_out = h1;
    *h2_out = h2;
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

    if (!isfinite(m) || m >= (double)SIZE_MAX) {
        return 0;
    }

    size_t m_bits = (size_t)ceil(m);
    size_t k_hashes = (size_t)llround(k);
    if (k_hashes == 0) {
        k_hashes = 1;
    }

    size_t bytes = (m_bits + 7u) / 8u;
    if (bytes == 0 || bytes > (SIZE_MAX / sizeof(uint8_t))) {
        return 0;
    }

    *out_m_bits = m_bits;
    *out_k_hashes = k_hashes;
    return 1;
}

static void bloom_filter_add_method(bloom_filter *self, const void *element, size_t len) {
    bloom_filter_impl *impl = bloom_filter_impl_from_self(self);
    if (!impl || !impl->bits || impl->m_bits == 0 || impl->k_hashes == 0) {
        return;
    }
    if (len > 0 && !element) {
        return;
    }

    uint64_t h1, h2;
    fnv1a64_dual(element, len, &h1, &h2);
    if (h2 == 0) {
        h2 = BLOOM_H2_FALLBACK;
    }

    for (size_t i = 0; i < impl->k_hashes; i++) {
        size_t idx = bloom_index(h1, h2, i, impl->m_bits);
        bloom_set_bit(impl->bits, idx);
    }

    impl->items_added++;
}

static bool bloom_filter_might_contain_method(const bloom_filter *self, const void *element, size_t len) {
    bloom_filter_impl *impl = bloom_filter_impl_from_self(self);
    if (!impl || !impl->bits || impl->m_bits == 0 || impl->k_hashes == 0) {
        return false;
    }
    if (len > 0 && !element) {
        return false;
    }

    uint64_t h1, h2;
    fnv1a64_dual(element, len, &h1, &h2);
    if (h2 == 0) {
        h2 = BLOOM_H2_FALLBACK;
    }

    for (size_t i = 0; i < impl->k_hashes; i++) {
        size_t idx = bloom_index(h1, h2, i, impl->m_bits);
        if (!bloom_get_bit(impl->bits, idx)) {
            return false;
        }
    }
    return true;
}

static void bloom_filter_clear_method(bloom_filter *self) {
    bloom_filter_impl *impl = bloom_filter_impl_from_self(self);
    if (!impl || !impl->bits || impl->m_bits == 0) {
        return;
    }

    size_t bytes = (impl->m_bits + 7u) / 8u;
    memset(impl->bits, 0, bytes);
    impl->items_added = 0;
}

static double bloom_filter_current_fpp_method(const bloom_filter *self) {
    bloom_filter_impl *impl = bloom_filter_impl_from_self(self);
    if (!impl || impl->m_bits == 0 || impl->k_hashes == 0 || impl->items_added == 0) {
        return 0.0;
    }

    const double m = (double)impl->m_bits;
    const double k = (double)impl->k_hashes;
    const double n = (double)impl->items_added;
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

static void bloom_filter_free_method(bloom_filter *self) {
    if (!self) {
        return;
    }

    bloom_filter_impl *impl = bloom_filter_impl_from_self(self);
    if (impl) {
        free(impl->bits);
        impl->bits = NULL;
        free(impl);
        self->impl = NULL;
    }

    free(self);
}

bloom_filter *create_bloom_filter(int expected_elements, double false_positive_rate) {
    size_t m_bits = 0;
    size_t k_hashes = 0;
    if (!bloom_compute_params(expected_elements, false_positive_rate, &m_bits, &k_hashes)) {
        return NULL;
    }

    bloom_filter *out = (bloom_filter *)calloc(1, sizeof(bloom_filter));
    if (!out) {
        return NULL;
    }

    bloom_filter_impl *impl = (bloom_filter_impl *)calloc(1, sizeof(bloom_filter_impl));
    if (!impl) {
        free(out);
        return NULL;
    }

    size_t bytes = (m_bits + 7u) / 8u;
    impl->bits = (uint8_t *)calloc(bytes, 1);
    if (!impl->bits) {
        free(impl);
        free(out);
        return NULL;
    }

    impl->m_bits = m_bits;
    impl->k_hashes = k_hashes;
    impl->expected_elements = (size_t)expected_elements;
    impl->target_fpp = false_positive_rate;
    impl->items_added = 0;

    out->impl = impl;
    out->add = bloom_filter_add_method;
    out->might_contain = bloom_filter_might_contain_method;
    out->clear = bloom_filter_clear_method;
    out->current_fpp = bloom_filter_current_fpp_method;
    out->free = bloom_filter_free_method;

    return out;
}
