# Containers

This page documents the internal layouts, algorithms, and operational costs for the general-purpose containers exposed through `include/list.h`, `include/queue.h`, `include/heap.h`, `include/stack.h`, and `include/map.h`.

## Lists

### Shared interface
- `create_list(ListType type, int capacity, comparator cmp)` returns a `list` handle whose function table provides `insert`, `get`, `remove`, `size`, and `free` pointers implemented by the selected backend (`src/list/list.c`).
- Every list stores opaque `void *` payloads. Ownership stays with the caller; destructors only release internal buffers.
- Index validation happens before pointer arithmetic. Out-of-range indexes are ignored to keep APIs tolerant of user input.
- `array_list_active_buffer_count()` (exposed from `src/list/array_list.c`) reports how many array-list buffers are live, aiding diagnostics when embedding the library.

| Operation | ARRAY_LIST | LINKED_LIST | SORTED_LIST |
|-----------|------------|-------------|-------------|
| `insert`  | O(n) because tail elements shift with `memmove` | O(n) traversal + O(1) splicing | O(log n) comparisons via binary search + O(n) shift |
| `get`     | O(1) random access | O(n) traversal (walk from nearer end) | O(1) by index |
| `remove`  | O(n) shift | O(n) traversal + O(1) unlink | O(n) shift |
| `size`    | O(1) | O(1) | O(1) |
| Growth    | Capacity doubles under `ensure_capacity` | Unbounded (node-per-insert) | Capacity doubles under `ensure_capacity` |

#### Array list (`ARRAY_LIST`)
- Backing store: contiguous `void **items` buffer plus `size`/`capacity` integers (`src/list/array_list.c`). Capacity doubles with `realloc` when `size == capacity`; failure leaves the array unchanged to avoid losing data.
- Inserts call `memmove` to open a single-slot gap before writing the payload. Deletions shift the tail left, shrinking `size` without altering `capacity`.
- The structure never shrinks; repeated removals leave spare capacity to keep future inserts amortized O(1).

#### Doubly linked list (`LINKED_LIST`)
- Implementation state (`linked_list_impl` in `src/list/linked_list.c`) stores `head`, `tail`, and `size`. Nodes carry `data`, `next`, and `prev` fields.
- Index-based insertion walks from `head` or `tail` depending on which side is closer to the target index, reducing comparisons to ≈`n/2` on average.
- Removing a node relinks neighbors and frees the node immediately. Since payload pointers are caller-owned, the list never touches the stored data.
- Cleanup traverses once from `head`, freeing nodes before the wrapper to survive partial construction failures.

#### Sorted array list (`SORTED_LIST`)
- Creation clamps `capacity` to at least four slots and fails if `cmp == NULL` (see `create_sorted_list` in `src/list/sorted_list.c`).
- `insert` ignores the caller-provided index. `find_insert_position` performs binary search to locate the insertion site while allowing duplicate keys; duplicates land after existing equals, preserving stability relative to insertion order.
- Deletes and lookups use direct indexing. The buffer doubles when full to maintain logarithmic search costs.

## Stacks

- `create_stack(StackType type)` (`src/stack/stack.c`) wraps the list abstractions to deliver LIFO semantics without duplicating storage logic.
- Array-backed stacks (`ARRAY_STACK`) push to the dynamic array tail (`array_stack_push`) and pop from the last index. Memory reuse follows the underlying array list policy.
- Linked stacks (`LINKED_STACK`) push/pop at the head of a doubly linked list to keep operations O(1) without reallocations.
- `stack_top` branches on the active push function to read the correct end (tail for array stacks, head for linked stacks), ensuring consistent results regardless of backend.

## Queues

### Linked FIFO queue (`QUEUE_TYPE_NORMAL`)
- `create_linked_queue` (`src/queue/linked_queue.c`) allocates a `queue` struct with `front`, `rear`, and `length` fields. Each `enqueue` allocates a `queue_entry` node that becomes the new tail; `dequeue` releases the head node.
- Empty queue detection is O(1) because `front == NULL` indicates exhaustion. Reinitialization resets pointers without freeing the wrapper, allowing reuse.

### Heap-backed priority queue (`QUEUE_TYPE_PRIORITY`)
- `create_heap_queue` (`src/queue/heap_queue.c`) wraps a heap instance produced by `create_heap`. The comparator passed to the queue constructor is forwarded to the heap and governs priority ordering.
- `priority_enqueue` delegates to `heap->put`, inheriting the heap’s O(log n) insert cost. `priority_dequeue` checks for emptiness before calling `heap->pop`.
- Size queries and emptiness checks proxy to the heap, so the queue reflects dynamic resizing performed by the underlying heap.
- `priority_free` releases the heap then frees the queue wrapper; payloads remain untouched.

## Heaps

`create_heap` (`src/heap/heap.c`) chooses between binary and Fibonacci implementations based on the requested `heap_type`. Both implementations treat a larger comparator return value as higher priority (max-heap semantics by default).

### Binary heap
- Data layout (`src/heap/binary_heap.c`): contiguous `void **data` array indexed using `parent = (i - 1) / 2`, `left = 2i + 1`, and `right = 2i + 2`.
- Insertion path: append at `data[size]`, call `sift_up` to bubble the item toward the root while the comparator prefers the child, then increment `size`.
- Removal path: capture the root, move the last element to index 0, decrement `size`, and `sift_down` to restore heap order. The algorithm picks the larger child to preserve max-heap behavior.
- Capacity management doubles the backing array when full. The heap never shrinks automatically; callers may reuse the structure for bursty workloads without reallocations.
- Complexity: `put` and `pop` are O(log n); `peek` and `size` are O(1). Memory footprint is `O(capacity)`.

### Fibonacci heap
- Node structure (`fib_node` in `src/heap/fibonacci_heap.h`): circular doubly linked root list with child lists per node, storing `degree`, `parent`, `child`, and `mark` fields.
- `fibonacci_heap_put` inserts new nodes into the root list in O(1) and updates the `min` pointer when the comparator prefers the new element.
- `fibonacci_heap_pop` removes the current minimum, merges its children into the root list, and calls `consolidate`. Consolidation scans up to 45 degree buckets (`fib_node* A[45]`)—enough for heaps below `φ^45` elements (~10⁹)—linking trees of equal degree until every degree slot is unique.
- Decrease-key and cascading cuts are not implemented yet, so the amortized bounds are: `insert` O(1), `pop` O(log n). The absence of decrease-key keeps `mark` mostly unused but preserves the structure required for future extensions.
- Memory cleanup recurses through each circular child list (`free_fib_node`) before freeing the heap wrapper and outer `heap` handle.

## Maps (Hash Tables)

### Core structure
- `create_hash_map` (`src/map/hash_map.c`) returns a `map` handle with a bucket array (`map_entry **buckets`) sized to at least `INITIAL_CAPACITY` (20). Each bucket holds a singly linked list of `map_entry { key, data, next }` nodes for separate chaining.
- `key_compare` (typedef `comparator` from `include/types.h`) must define equality semantics. `hash_get` and `hash_remove` first compare pointer identity, then fall back to `key_compare` when the pointers differ.
- NULL keys are legal and are always mapped to bucket zero. This ensures sentinel keys can be stored without dereferencing.

### Operations and resizing
- `hash_insert` locks the optional mutex (for `HASH_TABLE`), evaluates the load factor (`size / capacity`), and doubles the bucket array via `resize_and_rehash` when the ratio exceeds `LOAD_FACTOR` (0.75).
- Rehashing allocates a new bucket array, recomputes each entry’s index with the current `hash_func`, and relinks nodes without reallocating them—minimizing heap churn.
- Insertion walks the target chain to locate an existing key; if found, it overwrites `data` in place. Otherwise, it allocates a new `map_entry` and pushes it to the bucket head (LIFO order), keeping O(1) insertion absent resizing.
- `hash_get` computes the bucket index, walks the chain, and returns the stored `data` pointer. `hash_remove` unlinks the matching node, frees it, decrements `size`, and returns the payload pointer for caller-managed cleanup.
- `hash_free` iterates all buckets, frees each node, destroys the optional mutex, and releases the map wrapper.

### Hash function options
All hash functions accept `(void *key, int capacity)` and return an index in `[0, capacity)`. The library exposes the following implementations (see `src/map/map.c`):

| Function | Strategy | When to use |
|----------|----------|-------------|
| `bernstein_hash` | djb2 (`hash * 33 + c`) | Default choice for short ASCII keys; low arithmetic cost and good dispersion. |
| `fnv1a_hash` | XOR then multiply by 16777619 | Structured binary inputs or longer strings needing stronger avalanche behavior. |
| `xor_hash` | Mix of shifts and XOR | Keys with predictable prefixes; additional mixing reduces clustering. |
| `rotational_hash` | Rotate-then-XOR | Variable-length records; rotation balances influence of earlier and later bytes. |
| `additive_hash` | Plain summation | Tiny key sets where speed outweighs distribution quality; expect higher collision rates. |

Select a function by passing it to `create_map`. When omitted, `bernstein_hash` is used. Custom hash functions can be provided via the `hash_func_t` typedef.

### Thread safety
- `create_map` with `HASH_TABLE` allocates a `pthread_mutex_t` and stores it in `map->lock`. All mutating operations (`put`, `remove`) and lookups (`get`) lock and unlock around critical sections.
- `HASH_MAP` omits the lock for zero-overhead single-threaded use. Clients needing reader/writer semantics must wrap the API externally.

### Complexity summary
- Expected average-case costs with well-distributed hashes: `put`, `get`, and `remove` are O(1).
- Worst-case (all keys collide): operations degrade to O(n) due to chain traversal.
- Space usage tracks `capacity + size` because resizing leaves slack buckets to maintain the 0.75 load factor target.
