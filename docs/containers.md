# Containers

## Lists
### Common contract
- `create_list` selects an implementation based on `ListType` and wires function pointers for `insert`, `get`, `remove`, `size`, and `free` so callers interact through a uniform API.
- Payload ownership is external. Removing an item unlinks the node or shifts slots but never frees the pointer stored by the user.

### Array list (`ARRAY_LIST`)
- Backed by a contiguous `void **` buffer stored in `array_list_impl`. Capacity doubles via `realloc` when the write pointer reaches the end, giving amortized O(1) growth for append-heavy workloads.
- `insert` calls `memmove` to open a gap before writing the new element; worst-case cost is O(n) when inserting at the head.
- `remove` also uses `memmove` to collapse the gap. Reads are O(1) because `get` dereferences the array directly.
- A static counter `active_item_buffers` tracks allocated backing arrays for diagnostic purposes.

### Linked list (`LINKED_LIST`)
- Doubly linked nodes (`linked_list_node`) carry `data`, `next`, and `prev` pointers. The implementation stores `head`, `tail`, and `size` inside `linked_list_impl`.
- Insertion chooses the nearest end when walking to the target index to cap traversal at roughly n/2 comparisons. Head and tail insertions update sentinel pointers without traversal.
- Removal fixes neighbor pointers before freeing the node, ensuring O(1) deletion when the node reference is known after traversal.

### Sorted array list (`SORTED_LIST`)
- Requires a comparator at creation time; calls fail if `cmp == NULL`.
- Maintains sorted order by binary searching (`find_insert_position`) for the insertion index and then shifting the tail with `memmove`. Lookup is O(1) by index, insert/remove remain O(n) due to shifting.
- Capacity clamps to at least four slots and doubles when the array is full.

## Queues
### Linked FIFO queue
- Implements an unbounded singly linked list of `queue_entry` nodes. `enqueue` appends at `rear`, `dequeue` pops from `front`, both O(1).
- `initialize` resets `front`, `rear`, and `length` so queues can be reused after draining.

### Heap-backed priority queue
- Wraps a heap created through `create_heap`. The queue delegates `enqueue` to `heap->put` and `dequeue` to `heap->pop`, exposing a priority queue while keeping the `queue` vtable consistent.
- Size queries forward to the heap, and the comparator supplied at construction time controls ordering.

## Heaps
### Binary heap
- Stores elements in a dynamic array. When `size == capacity`, capacity doubles and `realloc` migrates elements.
- `put` inserts at the end and calls `sift_up` to restore heap order by comparing parents and swapping until the comparator no longer prefers the child.
- `pop` replaces the root with the last element, decrements `size`, and `sift_down`s by repeatedly selecting the largest child (max-heap semantics) until the heap property is restored.
- Time bounds: `put` and `pop` are O(log n); `peek` and `size` are O(1).

### Fibonacci heap
- Each heap node (`fib_node`) participates in a circular doubly linked root list. Nodes track `degree`, `parent`, `child`, and a Boolean `mark` flag.
- Inserts splice the new node into the root list in O(1). The minimum pointer updates if the new key outranks the previous root.
- `pop` removes the current minimum, promotes all of its children to the root list, and calls `consolidate`. Consolidation uses a fixed array of 45 buckets (`A[45]`) to link roots of equal degree, reflecting the upper bound on degrees for heaps with fewer than `φ^45` nodes (~10^9).
- Cascading cuts are not yet implemented; decrease-key is absent, so marked nodes remain unused but keep the structure compatible with future extensions.
- Amortized costs: `insert` O(1), `pop` O(log n) because consolidation links at most logarithmically many roots.
- Memory reclamation walks circular sibling lists recursively to release children before freeing the heap wrapper.

## Stacks
- Array-backed stacks reuse the array list to provide push/pop at the tail; linked stacks push/pop the head of a linked list. Both expose `top`, `is_empty`, `size`, and `free` consistent with the list contract.

## Maps
### Structure
- The map uses separate chaining. `map_entry` nodes store `key`, `data`, and `next` pointer. `buckets` points to an array of bucket heads with initial capacity clamped to `INITIAL_CAPACITY` (20).
- `create_hash_map` assigns the function pointers for `put`, `get`, `remove`, and `free` and either wires a caller-supplied hash function or defaults to `bernstein_hash`.
- When `map_type` is `HASH_TABLE`, the factory allocates a `pthread_mutex_t` and wraps all mutations and lookups with coarse-grained locking. Non-thread-safe instances leave `lock == NULL` for zero-overhead single-threaded use.

### Collision handling and resizing
- `hash_insert` locks (if enabled), recomputes the load factor `size / capacity`, and triggers `resize_and_rehash` when the ratio exceeds 0.75. Resizing doubles `capacity`, allocates a new bucket array, and reuses existing `map_entry` nodes by recomputing their indices.
- Collisions append new nodes at the head of the bucket’s linked list. Updating an existing key overwrites `data` in-place without reallocating the node.
- `hash_get` hashes the key (treating NULL as bucket zero), walks the chain, and returns the stored `data` pointer. `hash_remove` fixes the singly linked chain before freeing the removed node and decrementing `size`.
- Deallocation iterates every bucket, frees each `map_entry`, destroys the optional mutex, and finally frees the table.

### Built-in hash functions
- **Bernstein (djb2)**: multiplies the accumulator by 33 and adds each byte. Provides good distribution for short ASCII keys with minimal arithmetic.
- **FNV-1a**: XORs each byte into the hash and multiplies by the FNV prime (16777619), trading a larger constant for improved avalanche properties on structured binary data.
- **XOR folding**: Mixes left and right shifts with byte addition to decorrelate simple patterns; useful when inputs exhibit local repetition.
- **Rotational**: Rotates the accumulator by combining left and right shifts before XORing the next byte; balances dispersion and cost for variable-length strings.
- **Additive**: Simple byte summation; fastest but lowest quality, intended for tiny key sets where collisions are tolerable.
- All functions short-circuit on `key == NULL`, forcing such entries into bucket zero so sentinel keys remain retrievable.
