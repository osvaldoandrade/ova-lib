# Sorting Utilities

## Sorter factory
`create_sorter` builds a lightweight object that records the comparator and wires helper callbacks. Quicksort is exposed through the `sort` entry, while `swap`, `shuffle`, `reverse`, `binary_search`, `copy`, `min`, `max`, and `min_max` compose higher-level operations. Because the sorter operates on the abstract `list` interface, the same algorithm can manipulate array lists, linked lists, or custom implementations that follow the contract.

## Element swapping semantics
`sorter_swap` reads both targeted elements, removes them from the list, and reinserts them at the opposite indices. This approach avoids assuming random-access writes on the underlying storage and preserves invariants for list implementations that might track metadata inside their `insert` and `remove` calls. Each swap performs two removals and two insertions; array lists therefore pay O(n) per swap, whereas linked lists spend O(n) on traversal plus O(1) on relinking.

## Iterative quicksort
`sorter_quick` implements a non-recursive quicksort that relies on an explicit stack sized at twice the list length to store pending subranges. Partitioning chooses the high index as the pivot. Elements less than or equal to the pivot swap forward, and the pivot finally moves into its resolved position. Average runtime remains O(n log n), while worst-case behavior reaches O(n²) when the list is already sorted and the pivot degenerates. The algorithm remains safe under re-entrancy because it allocates and frees its working stack inside each call.

## Randomization and reversal
`collections_shuffle` executes a Fisher–Yates shuffle by swapping each element with a random index in `[0, i]`. Because the operation routes through the list API, invariants from custom implementations remain intact. `collections_reverse` performs mirrored swaps until the indices meet, delivering an in-place reversal with linear-time cost.

## Search and copy helpers
`collections_binary_search` assumes the list is sorted according to the sorter’s comparator. It performs an iterative binary search that returns the matching index or −1. Array lists observe O(log n) runtime, while linked lists degrade to O(n log n) because each index access traverses nodes. `collections_copy` iterates through the source list and inserts each element into the destination at the same index. The destination must already expose an `insert` that grows storage on demand or have adequate capacity.

## Extremum calculations
`collections_min_max` scans the list in pairs, comparing members to reduce comparator calls before updating global minima and maxima. Odd-length lists handle the final element separately. `collections_min` and `collections_max` perform full linear scans and return NULL for empty lists, matching the behavior of the combined routine.
