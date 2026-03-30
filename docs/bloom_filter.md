# Bloom Filter

The bloom filter module provides one space-bounded membership structure with two tuning inputs: expected element count and target false-positive rate.

## Construction

```c
bloom_filter *create_bloom_filter(int expected_elements, double false_positive_rate);
```

The constructor returns `NULL` when `expected_elements <= 0`, when the false-positive rate is not in the open interval `(0, 1)`, or when the derived bit-array size would overflow the allocator.

## Data Model

The implementation stores:

- `m_bits`, the bit-array size
- `k_hashes`, the number of derived hash probes
- `items_added`, the number of insert calls applied to the instance

The bit-array size and hash count are derived from the standard Bloom-filter equations. The implementation uses two seeded 64-bit FNV-1a passes and double hashing to derive the `k` probe positions.

## API Semantics

`bloom_filter_add` treats the payload as an opaque byte sequence of length `len`. `bloom_filter_might_contain` returns `false` only when the item is definitely absent. A `true` result means the item may be present.

`bloom_filter_clear` resets the bit array and the insertion counter. `bloom_filter_current_fpp` estimates the current false-positive probability from the live `m`, `k`, and `n` values in the instance.

The filter never stores or frees the caller payload.
