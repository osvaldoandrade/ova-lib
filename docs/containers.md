# Containers

This page covers the shipped linear containers, heaps, maps, and deque. The graph, set, trie, matrix, and bloom-filter modules have their own pages because their ownership and return rules differ.

## Lists

`create_list` selects one of 3 implementations behind the same method table: `ARRAY_LIST`, `LINKED_LIST`, or `SORTED_LIST`. Every list exposes `insert`, `get`, `remove`, `size`, and `free`.

`ARRAY_LIST` stores a `void **` buffer and grows geometrically. Appending by inserting at `size` is amortized constant time. Middle insertion and removal shift elements with `memmove`, so they cost linear time.

`LINKED_LIST` stores nodes with `prev` and `next` pointers. Insertion or removal at a known position avoids shifting, but `get` still pays traversal cost. The implementation chooses the nearer end when traversing by index.

`SORTED_LIST` keeps the buffer ordered after every insertion. The comparator is required. The `index` argument passed to `insert` is ignored by design, because the implementation places the item where the comparator says it belongs.

## Queues

`create_queue(QUEUE_TYPE_NORMAL, ...)` builds a linked FIFO queue. The `capacity` argument is ignored on that path. `enqueue` appends at the rear, `dequeue` removes from the front, `is_empty` checks length, and `free` releases queue nodes only.

`create_queue(QUEUE_TYPE_PRIORITY, capacity, cmp)` builds a priority queue on top of a binary heap. The comparator is required for meaningful ordering. `enqueue` forwards to the heap, `dequeue` pops the current top-priority payload, and `size` forwards to the heap size.

## Heaps

`create_heap(BINARY_HEAP, capacity, cmp)` builds an array-backed heap. The top element is whichever payload the comparator ranks highest. `put` and `pop` cost logarithmic time. `peek` and `size` cost constant time.

`create_heap(FIBONACCI_HEAP, capacity, cmp)` ignores the capacity hint and builds the pointer-based heap variant. This API adds `put_with_handle`, `decrease_key`, and `delete_node`. The handle returned by `put_with_handle` is an internal node reference. It stops being valid after `delete_node`, after a matching removal, or after `free`.

The current priority queue wrapper always uses the binary heap. If you need `decrease_key`, use the heap API directly.

## Stacks

`create_stack(ARRAY_STACK)` wraps an array list. Push and pop operate at the tail. `create_stack(LINKED_STACK)` wraps a linked list and operates at the head. Both expose `push`, `pop`, `top`, `is_empty`, `size`, and `free`.

## Maps

`create_map` selects between `HASH_MAP` and `HASH_TABLE`. Both use separate chaining with `map_entry` nodes. `HASH_TABLE` adds one mutex. `HASH_MAP` does not.

The function signature is:

```c
map *create_map(map_type type, int capacity, int (*hash_func)(void *, int), comparator compare);
```

Three details matter in practice.

The first is capacity. Values below `INITIAL_CAPACITY` are raised to `20`.

The second is hashing. Passing `NULL` selects `bernstein_hash`. The header also exports `fnv1a_hash`, `xor_hash`, `rotational_hash`, and `additive_hash`.

The third is equality. The map stores the comparator pointer and uses it when traversing collisions. A non-`NULL` comparator is the safe path for all keyed lookups except pointer-identity-only scenarios.

When the load factor exceeds `0.75`, the table doubles and rehashes existing nodes.

## Deque

`create_deque(capacity)` builds a circular buffer. Non-positive capacities fall back to `16`. `deque_push_front`, `deque_push_back`, `deque_pop_front`, and `deque_pop_back` all operate at the ends of the buffer. `deque_get` provides indexed reads relative to the logical front. Growth doubles the backing array and rewrites elements in logical order.
