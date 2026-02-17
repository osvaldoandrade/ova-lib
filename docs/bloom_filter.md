# Bloom Filter

ova-lib includes a Bloom filter implementation for space-efficient set membership testing with a tunable false-positive rate.

Bloom filters have:

- No false negatives: if `bloom_filter_might_contain` returns `false`, the element is definitely not present.
- Possible false positives: if it returns `true`, the element might be present.

## API

Public declarations live in `include/bloom_filter.h`:

```c
bloom_filter* create_bloom_filter(int expected_elements, double false_positive_rate);
void bloom_filter_free(bloom_filter *bf);

void bloom_filter_add(bloom_filter *bf, const void *element, size_t len);
bool bloom_filter_might_contain(const bloom_filter *bf, const void *element, size_t len);

void bloom_filter_clear(bloom_filter *bf);
double bloom_filter_current_fpp(const bloom_filter *bf);
```

## Sizing

The filter chooses the bit-array size (`m`) and number of hash functions (`k`) using standard formulas:

- `m = -n * ln(p) / (ln(2)^2)`
- `k = (m / n) * ln(2)`

Where:

- `n` is `expected_elements`
- `p` is `false_positive_rate`

## Hashing

The implementation uses seeded FNV-1a hashes and a double-hashing scheme to generate `k` indices into the `m`-bit array.

## Semantics

- Elements are treated as raw bytes (`element`, `len`). This supports strings and arbitrary binary keys.
- The Bloom filter does not store or free elements.
- `bloom_filter_clear` resets all bits and counters.
- `bloom_filter_current_fpp` returns an estimate based on the number of insert calls performed on the instance:
  `p ~= (1 - exp(-(k*n)/m))^k`.

## Example

```c
#include "bloom_filter.h"
#include <string.h>

int main(void) {
    bloom_filter *bf = create_bloom_filter(1000, 0.01);

    const char *key = "hello";
    bloom_filter_add(bf, key, strlen(key));

    int ok = bloom_filter_might_contain(bf, key, strlen(key));

    bloom_filter_free(bf);
    return ok ? 0 : 1;
}
```

