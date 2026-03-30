# Sorting Helpers

The sorting module wraps list operations in a `sorter` object created by `create_sorter(list *data, comparator cmp)`. The `data` argument is ignored by the current implementation; the comparator is what matters.

## What the Sorter Does

The sorter installs 8 helpers:

- `sort`
- `shuffle`
- `reverse`
- `binary_search`
- `swap`
- `copy`
- `min`
- `max`
- `min_max`

`sort` is an iterative quicksort over the public `list` interface. `shuffle` uses Fisher-Yates. `reverse` swaps mirrored pairs. `binary_search` assumes the list is already sorted according to the same comparator. `copy` re-inserts each source element into the destination list.

## Costs

The choice of underlying list still matters because every helper routes through `insert`, `get`, and `remove`.

On an `ARRAY_LIST`, quicksort behaves as you would expect from indexed access. On a `LINKED_LIST`, the same algorithm pays traversal cost on every indexed read and write, so asymptotic quicksort structure stays the same while the constant factor grows.

## Lifetime Note

The public header currently exposes `create_sorter` but no matching destructor. If you use this helper in long-running code, account for the missing destroy path in your application layer.
