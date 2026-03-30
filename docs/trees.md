# Balanced Trees

The tree module exposes 2 balanced binary-search-tree variants through one API: `TREE_AVL` and `TREE_RED_BLACK`. Both store caller-owned key and value pointers and order keys with the comparator passed to `create_tree`.

## Core Behavior

`tree_insert` stores a key-value pair when the key is new. When the comparator reports equality with an existing key, the tree updates the stored value and keeps the original key pointer.

`tree_search` returns the stored value pointer or `NULL`. `tree_delete` removes one key if present and does nothing if the key is absent. `tree_size` reports the number of stored pairs.

`tree_min` and `tree_max` return values, not keys. The same is true for `tree_predecessor`, `tree_successor`, and `tree_range_query`.

## Range and Traversal Results

`tree_range_query(low, high)` returns an allocated list of value pointers in ascending key order for the inclusive range `[low, high]`. The list container belongs to the caller. The payload pointers still belong to the caller or to the application objects already stored in the tree.

`tree_in_order_traverse` invokes `callback(key, value)` in ascending key order. This is the only public way to visit both keys and values without copying them out to a list first.

## Choosing a Variant

Pick `TREE_AVL` when lookup depth is the main concern. Pick `TREE_RED_BLACK` when you want the standard red-black balance tradeoff and you do not need to care about the difference. The public API and ownership rules are identical for both.
