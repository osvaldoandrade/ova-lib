# Sorting Utilities

## Sorter factory
- `create_sorter` builds a lightweight object storing the comparator and wiring helper callbacks. The quicksort routine is assigned to `sort`, while `swap`, `shuffle`, `reverse`, `binary_search`, `copy`, `min`, `max`, and `min_max` compose higher-level operations.
- The sorter operates on the abstract `list` interface, enabling the same algorithm to drive array lists, linked lists, or custom list implementations that respect the contract.

## Element swapping semantics
- `sorter_swap` reads both elements, removes them from the list, and reinserts them at the opposite indices. This approach avoids requiring random-access writes on the underlying storage and preserves invariants for list implementations that might track metadata on `insert`/`remove`.
- The cost is two removals and two insertions per swap; array lists pay O(n) per swap, while linked lists pay O(n) for traversal plus O(1) for relinking.

## Iterative quicksort
- `sorter_quick` implements non-recursive quicksort using an explicit stack sized at `2 * list->size(list)` integers to hold pending subranges. This sidesteps recursion limits and makes stack usage predictable.
- Partitioning chooses the high index as the pivot. Elements less than or equal to the pivot swap forward; finally the pivot is swapped into its final position. Average runtime is O(n log n), worst-case O(n²) if the list is already sorted and the pivot degenerates.
- The implementation is stable under re-entrancy because it allocates the working stack per invocation and frees it before returning.

## Randomization and reversal
- `collections_shuffle` performs Fisher–Yates by swapping each element with a random index in `[0, i]`. Because swaps route through the list API, all list invariants remain intact.
- `collections_reverse` swaps mirrored indices until they meet, producing an in-place reversal with O(n) swaps.

## Search and copy helpers
- `collections_binary_search` assumes the list is sorted according to the sorter’s comparator. It performs iterative binary search returning the matching index or −1. Runtime is O(log n) on array lists but degrades to O(n log n) for linked lists due to per-access traversal.
- `collections_copy` iterates through the source list and inserts each element into the destination at the same index. The destination must already have adequate capacity or a compatible `insert` that grows storage on demand.

## Extremum calculations
- `collections_min_max` scans the list in pairs, reducing comparator calls by comparing pair members first and then updating global min/max. Handles odd-length lists by checking the final element separately.
- `collections_min` and `collections_max` run linear scans returning the extremal elements. All three functions return NULL for empty lists.
