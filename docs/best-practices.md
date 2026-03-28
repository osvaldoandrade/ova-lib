# Usage Best Practices Guide

This guide provides practical recommendations for using ova-lib effectively. It covers structure selection, memory ownership, thread safety, error handling, performance, and common pitfalls.

---

## When to Use Which Data Structure

Choosing the right data structure is the single most impactful decision you can make. The table below provides quick guidance; the sections that follow give additional context.

| Use Case | Recommended Structure | Key Reason |
|---|---|---|
| Random access by index | `ARRAY_LIST` | O(1) get, cache-friendly contiguous layout |
| Frequent insertions/removals at both ends | `Deque` | O(1) amortized push/pop at front and back |
| Always-sorted collection | `SORTED_LIST` | Binary-search insertion, O(1) indexed read |
| Ordered key-value store (read-heavy) | AVL Tree | Stricter balance → marginally faster lookups |
| Ordered key-value store (write-heavy) | Red-Black Tree | Fewer rotations on insert and delete |
| LIFO (last-in, first-out) | Array Stack | Faster than linked stack due to cache locality |
| FIFO (first-in, first-out) | `QUEUE_TYPE_NORMAL` | O(1) enqueue/dequeue |
| Priority scheduling, simple insert+extract | Binary Heap / Priority Queue | Low constant factor, cache-friendly array |
| Priority queue with frequent key updates | Fibonacci Heap | O(1) amortized `decrease_key` |
| Graph algorithms (Dijkstra, Prim) | Fibonacci Heap | `decrease_key` dominates; O(1) amortized beats O(log n) |
| Unordered key-value lookup | `HASH_TABLE` | O(1) average `put`/`get`/`remove` |
| Concurrent key-value access | `HASH_TABLE` (thread-safe) | Built-in `pthread_mutex_t` protects all operations |
| Exact set membership | Hash Set | O(1) average `contains` |
| Space-efficient probabilistic membership | Bloom Filter | ~1.2 bytes/element at 1 % false-positive rate |
| Prefix search / autocomplete | Trie | O(m) lookup independent of dictionary size |
| Sparse graph traversal (BFS, DFS) | Adjacency-list Graph | Memory-efficient; O(V+E) traversal |
| Dense graph / matrix-based algorithms | Adjacency-matrix Graph | O(1) edge lookup; suits Floyd-Warshall |
| Small-to-medium matrix math | Matrix | Built-in multiply, transpose, determinant |

### Lists

- **`ARRAY_LIST`** — prefer this for any workload where you read by index more often than you insert or remove in the middle. Sequential iteration is especially fast because the contiguous layout allows the CPU prefetcher to work ahead.
- **`LINKED_LIST`** — only sensible when you insert or remove frequently at positions that you already hold pointers to, and you never access elements by index. At 10 000 elements, random index-based access on a linked list is roughly 800× slower than on an array list.
- **`SORTED_LIST`** — use when you need the collection to be sorted at all times and you read by index. Insertion uses binary search to find the slot then `memmove` to shift; it is O(n) in the worst case (head insertions) but O(1) per indexed read after that.

### Stacks and Queues

- Use the **array stack** (`ARRAY_STACK`) when elements are pushed and popped in tight loops. Contiguous storage keeps the top-of-stack warm in the L1 cache.
- Use the **linked stack** (`LINKED_STACK`) when stack depth is highly variable and you want to avoid the occasional doubling reallocation that the array stack performs.
- For FIFO queues, use `QUEUE_TYPE_NORMAL`. For priority queues, use `QUEUE_TYPE_PRIORITY` backed by a binary heap unless `decrease_key` is needed (then use a Fibonacci heap directly).

### Heaps

- **Binary Heap** — the default choice for priority queues. Insert and pop are both O(log n), the backing array keeps nodes close in memory, and there is no per-node pointer overhead.
- **Fibonacci Heap** — use only when your algorithm calls `decrease_key` frequently (e.g., Dijkstra's shortest-path, Prim's MST). The amortized O(1) `decrease_key` outweighs the higher constant factor for pop in those scenarios. If you only insert and pop, prefer the binary heap: at 100 000 elements binary-heap pop is ~3.5× faster (0.196 µs vs 0.686 µs).

### Maps and Sets

- Start with `HASH_TABLE` and the default Bernstein (`djb2`) hash. Switch to FNV-1a when keys are structured binary data; switch to the additive hash only for tiny key sets where collision quality does not matter.
- Use a `TREE_SET` (backed by a Red-Black tree) instead of a hash set when you need sorted iteration over set members or range queries.
- Use a `Bloom Filter` as a fast pre-filter in front of a more expensive lookup to eliminate obvious misses cheaply. Remember that it can report false positives but never false negatives.

---

## Memory Management Guidelines

ova-lib follows a clear ownership model: **the library owns its internal structures; the caller owns all payload pointers.**

### Core rules

1. **Always call the matching `free` / `destroy` function.** Every factory function has a corresponding destructor:

   ```c
   list *l = create_list(ARRAY_LIST, 16, NULL);
   /* use l */
   l->free(l);          /* releases the internal buffer and the list handle */
   ```

   ```c
   map *m = create_hash_map(HASH_TABLE, NULL, bernstein_hash);
   /* use m */
   m->free(m);          /* releases all bucket chains and the map handle */
   ```

2. **The library never frees your data.** Removing an element from a list, heap, or map only unlinks the internal node; the pointed-to data is not freed. Free your data separately before or after destroying the container:

   ```c
   for (int i = l->size(l) - 1; i >= 0; --i) {
       free(l->get(l, i));   /* free payload */
       l->remove(l, i);
   }
   l->free(l);
   ```

3. **Check `create_*` return values.** All factory functions return `NULL` when memory allocation fails. Dereferencing a `NULL` handle is undefined behaviour. See [Error Handling Patterns](#error-handling-patterns) below.

4. **Do not keep stale pointers to internal nodes.** Handles returned by `put_with_handle` (Fibonacci heap) become invalid after the corresponding node is removed or after `h->free(h)` is called. Discard them immediately after use.

5. **Account for geometric growth.** Array-backed structures (array list, sorted list, binary heap, deque) double their capacity on overflow. Peak memory is up to 2× the logical size. If memory is constrained, pre-size the container by passing the expected capacity to the factory and avoid unnecessary growth:

   ```c
   /* Pre-size to 1024 to avoid early reallocations */
   list *l = create_list(ARRAY_LIST, 1024, NULL);
   ```

6. **Matrices own their `data` 2-D array.** Call `matrix->destroy(matrix)` when done; do not free `matrix->data` manually.

---

## Thread-Safety Considerations

ova-lib components are **not thread-safe by default**. The only built-in concurrency support is in the hash map.

### Thread-safe hash map

Pass `HASH_TABLE` as the type to `create_hash_map`; the factory allocates a `pthread_mutex_t` and wraps every `put`, `get`, and `remove` with coarse-grained locking:

```c
map *m = create_hash_map(HASH_TABLE, my_key_cmp, my_hash_fn);
/* m->put, m->get, m->remove are now mutex-protected */
```

Single-threaded maps omit the mutex entirely; do not request a thread-safe map when only one thread accesses it, as the lock/unlock overhead is measurable.

### External synchronization for all other structures

For any other container accessed from multiple threads, protect every operation with an external lock:

```c
pthread_mutex_t list_lock = PTHREAD_MUTEX_INITIALIZER;

/* Writer thread */
pthread_mutex_lock(&list_lock);
l->insert(l, item, l->size(l));
pthread_mutex_unlock(&list_lock);

/* Reader thread */
pthread_mutex_lock(&list_lock);
void *item = l->get(l, 0);
pthread_mutex_unlock(&list_lock);
```

### Iteration is not safe under concurrent mutation

There is no built-in iterator API. If you must iterate while another thread may mutate the container, hold the lock for the full duration of the traversal, or take a snapshot first.

### Avoid sharing handles across threads without synchronization

Fibonacci-heap node handles returned by `put_with_handle` are raw pointers into the heap's internal state. Never pass a handle to another thread while the heap may also be mutated.

---

## Error Handling Patterns

### Always check factory return values

```c
list *l = create_list(ARRAY_LIST, 64, NULL);
if (!l) {
    fprintf(stderr, "create_list failed: out of memory\n");
    return -1;
}
```

```c
heap *h = create_heap(FIBONACCI_HEAP, 0, my_cmp);
if (!h) {
    /* clean up already-allocated resources before returning */
    l->free(l);
    return -1;
}
```

### Check `put_with_handle` before using the handle

```c
void *handle = h->put_with_handle(h, &value);
if (!handle) {
    /* insertion failed; do not call decrease_key with a NULL handle */
    return -1;
}
```

### Validate indices before calling `get` or `remove`

The list and deque implementations validate bounds internally, but calling with an out-of-range index will return `NULL` (lists) or trigger a no-op. Guard against this explicitly:

```c
int idx = compute_index();
if (idx >= 0 && idx < l->size(l)) {
    void *item = l->get(l, idx);
}
```

### Check the solver result before reading the solution

```c
if (s->solve(problem, &tableau) == OPTIMAL) {
    /* safe to read problem->solution and problem->z_value */
} else {
    /* handle INFEASIBLE or other non-optimal status */
}
```

### Never pass NULL comparators where required

`SORTED_LIST` and priority-queue-backed queues (`QUEUE_TYPE_PRIORITY`) require a non-NULL comparator. Passing `NULL` causes the factory to return `NULL`:

```c
list *s = create_list(SORTED_LIST, 8, my_cmp);  /* my_cmp must not be NULL */
```

---

## Performance Tips

### Size containers at construction time

The initial capacity hint passed to `create_list`, `create_heap`, and similar functions lets you avoid the first several reallocation events. Use it when you have a reasonable estimate:

```c
/* Processing 10 000 records — pre-size to avoid ~14 reallocations */
list *l = create_list(ARRAY_LIST, 10000, NULL);
```

### Prefer `ARRAY_LIST` over `LINKED_LIST` for sequential scans

At 10 000 elements, sequential `get` on an array list runs at 0.002 µs/op versus 6.377 µs/op on a linked list — a 3 189× difference caused by cache-friendly contiguous memory access.

### Use `DEQUE` when you need both ends

A deque combines the cache efficiency of a contiguous buffer with O(1) amortized push and pop at both ends. It is faster than a linked list for front/back operations and supports O(1) random access by index.

### Prefer `BINARY_HEAP` unless `decrease_key` is critical

Binary heap pop at 100 000 elements is ~3.5× faster than Fibonacci heap pop (0.196 µs vs 0.686 µs). The Fibonacci heap's advantage — O(1) amortized `decrease_key` — only pays off in algorithms that call `decrease_key` more often than `pop`, such as Dijkstra's algorithm on dense graphs.

### Choose the right hash function

| Hash Function | Best For |
|---|---|
| Bernstein (djb2) | Short ASCII strings (default) |
| FNV-1a | Binary or structured keys needing strong distribution |
| Rotational | Variable-length strings with mixed patterns |
| Additive | Tiny key sets where speed matters more than quality |
| XOR folding | Keys with locally repetitive byte patterns |

### Prefer adjacency lists for sparse graphs

The adjacency-matrix representation uses 8·V² bytes regardless of edge count. For a graph with 10 000 vertices and E ≪ V², this wastes hundreds of megabytes. Use `GRAPH_ADJACENCY_LIST` and switch to `GRAPH_ADJACENCY_MATRIX` only for dense graphs or algorithms like Floyd-Warshall that access every cell.

### Tune bloom filter parameters at creation time

Bloom filter false-positive rate and memory depend on the number of elements you plan to insert and the number of hash functions. Decide both at construction time — you cannot resize a bloom filter after creation.

### Avoid sorting through the abstract list interface for large collections

The built-in quicksort routes all swaps through the abstract `insert`/`remove` vtable calls. For large arrays, sorting a raw C array and then importing the result into an `ARRAY_LIST` is faster when you need maximum throughput.

---

## Common Pitfalls to Avoid

### Pitfall 1 — Ignoring `NULL` returns from factory functions

```c
/* Wrong */
list *l = create_list(ARRAY_LIST, 64, NULL);
l->insert(l, item, 0);   /* crash if l is NULL */

/* Correct */
list *l = create_list(ARRAY_LIST, 64, NULL);
if (!l) { /* handle error */ }
l->insert(l, item, 0);
```

### Pitfall 2 — Double-freeing payload data

Removing an element from a container does not free it. Freeing the container does not free the stored pointers either. Only free payloads once, and do it explicitly:

```c
/* Wrong: payload freed but still referenced in list */
free(my_item);
l->remove(l, 0);   /* internal node freed; my_item already freed → double-free risk */

/* Correct: remove first, then free */
l->remove(l, 0);
free(my_item);
```

### Pitfall 3 — Using a Fibonacci-heap node handle after removal

```c
void *handle = h->put_with_handle(h, &val);
h->pop(h);                     /* may remove the same node */
h->decrease_key(h, handle, &new_val);  /* undefined: handle is stale */
```

Once a node is popped or explicitly deleted, its handle is invalid. Set handles to `NULL` after use:

```c
h->delete_node(h, handle);
handle = NULL;
```

### Pitfall 4 — Using `decrease_key` to lower priority instead of raise it

Despite the name (which comes from min-heap literature), `decrease_key` **increases** the node's priority according to the heap's comparator. For a max-heap, pass a larger value; for a min-heap, pass a smaller value. Passing a value that reduces priority results in undefined heap ordering.

### Pitfall 5 — Forgetting that sorted-list insertion ignores the index argument

```c
list *s = create_list(SORTED_LIST, 8, int_cmp);
s->insert(s, &val, 3);  /* index 3 is ignored; element is placed by comparator */
```

The index parameter of `insert` on a `SORTED_LIST` is silently discarded. The element is always placed in its sorted position.

### Pitfall 6 — Calling unsupported operations on binary heaps

`decrease_key` and `delete_node` are `NULL` on binary-heap handles. Calling them causes a null-function-pointer dereference:

```c
heap *h = create_heap(BINARY_HEAP, 16, cmp);
void *handle = h->put_with_handle(h, &val);  /* returns NULL for binary heap */
h->decrease_key(h, handle, &new_val);        /* crashes: function pointer is NULL */
```

Use `FIBONACCI_HEAP` when you need `decrease_key` or `delete_node`.

### Pitfall 7 — Sharing unsynchronized containers across threads

Only the thread-safe hash map variant (`HASH_TABLE`) is safe for concurrent access. All other containers require external synchronization. Accessing a list, heap, or tree from multiple threads simultaneously without a lock corrupts internal pointers.

### Pitfall 8 — Treating bloom filter "not present" as definitive

A bloom filter can report false positives (saying an element is present when it is not) but never false negatives. Do not use a bloom filter as the sole gate for actions that must be exact:

```c
/* Wrong: bloom filter false positive can cause incorrect branch */
if (bloom->contains(bloom, key)) {
    process_record(key);   /* key might not actually exist */
}

/* Correct: use bloom filter only as a fast pre-filter */
if (bloom->contains(bloom, key) && map->get(map, key) != NULL) {
    process_record(key);
}
```

### Pitfall 9 — Calling `matrix->destroy` without initializing all cells

`create_matrix(rows, cols)` allocates the `data` array but does not zero-initialize cells. Reading uninitialized cells before writing them produces undefined values:

```c
matrix *m = create_matrix(3, 3);
double x = m->data[0][0];   /* indeterminate value */
m->data[0][0] = 1.0;        /* correct: write before read */
```

### Pitfall 10 — Mistaking `create_queue(QUEUE_TYPE_PRIORITY, capacity, NULL)` for a valid priority queue

A priority queue requires a comparator to order elements. Passing `NULL` returns `NULL` from the factory:

```c
/* Wrong */
queue *pq = create_queue(QUEUE_TYPE_PRIORITY, 64, NULL);  /* returns NULL */

/* Correct */
queue *pq = create_queue(QUEUE_TYPE_PRIORITY, 64, my_cmp);
if (!pq) { /* handle error */ }
```

---

## Quick-Reference Checklist

Use this checklist when introducing ova-lib into a project:

- [ ] Check every `create_*` / factory return value for `NULL` before use
- [ ] Match every `create_*` call with the corresponding `free` / `destroy` call
- [ ] Free payload data separately; the library never owns your data
- [ ] Use a thread-safe map (`HASH_TABLE`) for concurrent access; add an external mutex for all other containers
- [ ] Pre-size containers with the expected element count to reduce reallocations
- [ ] Use `ARRAY_LIST` for indexed workloads; reserve `LINKED_LIST` for pointer-chasing patterns
- [ ] Use `BINARY_HEAP` for simple priority queues; switch to `FIBONACCI_HEAP` only when `decrease_key` is hot
- [ ] Validate that comparators are non-NULL when creating `SORTED_LIST` or priority queues
- [ ] Nullify Fibonacci-heap node handles after `delete_node` or `pop`
- [ ] Use a bloom filter only as a pre-filter; always confirm membership with an exact lookup
