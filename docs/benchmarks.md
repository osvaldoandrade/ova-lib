# Performance Benchmarks

## Methodology

All benchmarks were run under the following conditions:

- **Compiler:** GCC 13.3.0
- **Flags:** `-O2` (standard release optimization)
- **Platform:** Linux x86\_64
- **Timer:** `clock_gettime(CLOCK_MONOTONIC)`, nanosecond precision
- **Metric:** microseconds per operation (µs/op) unless noted otherwise
- **Workload:** each size point inserts N elements, then exercises the target operation N times

Results reflect amortized per-operation cost. Heap and deque timings include dynamic resizing overhead. Sorted-list insertions maintain sorted order via binary search. Linked-list get and remove costs include traversal from the nearest end.

---

## Time Complexity Reference

### Core Containers

| Operation | Array List | Linked List | Sorted List | Deque |
|---|---|---|---|---|
| Insert (append) | O(1) amortized | O(1) tail | O(n) | O(1) amortized |
| Insert (at index) | O(n) | O(n) | O(n) | — |
| Get (by index) | O(1) | O(n) | O(1) | O(1) |
| Remove (by index) | O(n) | O(n) | O(n) | — |
| Push front/back | — | O(1) | — | O(1) amortized |
| Pop front/back | — | O(1) | — | O(1) amortized |
| Size | O(1) | O(1) | O(1) | O(1) |

### Stacks and Queues

| Operation | Array Stack | Linked Stack | FIFO Queue | Priority Queue |
|---|---|---|---|---|
| Push / Enqueue | O(1) amortized | O(1) | O(1) | O(log n) |
| Pop / Dequeue | O(1) | O(1) | O(1) | O(log n) |
| Peek / Top | O(1) | O(1) | O(1) | O(1) |
| Size | O(1) | O(1) | O(1) | O(1) |

### Heaps

| Operation | Binary Heap | Fibonacci Heap |
|---|---|---|
| Put (insert) | O(log n) | O(1) amortized |
| Pop (extract-min/max) | O(log n) | O(log n) amortized |
| Peek | O(1) | O(1) |
| decrease\_key | O(log n) | O(1) amortized |
| delete\_node | O(log n) | O(log n) amortized |
| Size | O(1) | O(1) |

### Balanced Trees

| Operation | AVL Tree | Red-Black Tree |
|---|---|---|
| Insert | O(log n) | O(log n) |
| Search | O(log n) | O(log n) |
| Delete | O(log n) | O(log n) |
| Min / Max | O(log n) | O(log n) |
| Predecessor / Successor | O(log n) | O(log n) |
| Range query | O(log n + k) | O(log n + k) |
| In-order traversal | O(n) | O(n) |

### Hash Map and Set

| Operation | Hash Map (avg) | Hash Map (worst) | Hash Set | Tree Set |
|---|---|---|---|---|
| Put / Add | O(1) | O(n) | O(1) avg | O(log n) |
| Get / Contains | O(1) | O(n) | O(1) avg | O(log n) |
| Remove | O(1) | O(n) | O(1) avg | O(log n) |
| Union / Intersection / Difference | — | — | O(n + m) | O(n + m) |

### Other Structures

| Operation | Trie | Bloom Filter | Graph (Adj. List) | Graph (Adj. Matrix) |
|---|---|---|---|---|
| Insert / Add | O(m) | O(k) | O(1) edge | O(1) edge |
| Search / Contains | O(m) | O(k) | — | — |
| Delete | O(m) | — | O(degree) | O(1) |
| Prefix check | O(m) | — | — | — |
| BFS / DFS | — | — | O(V + E) | O(V²) |
| Dijkstra | — | — | O((V+E) log V) | O(V² log V) |
| Bellman-Ford | — | — | O(V·E) | O(V·E) |
| Floyd-Warshall | — | — | O(V³) | O(V³) |

*m = key/word length, k = number of hash functions, V = vertices, E = edges*

### Sorting and Search

| Operation | Complexity |
|---|---|
| Quicksort (average) | O(n log n) |
| Quicksort (worst, sorted input) | O(n²) |
| Binary search | O(log n) on array list; O(n log n) on linked list (each of O(log n) probes traverses O(n) nodes) |
| Fisher-Yates shuffle | O(n) |
| Min/Max (pair scan) | O(n), 3n/2 comparisons |

### Matrix and Linear Algebra

| Operation | Complexity |
|---|---|
| Addition / Subtraction | O(m·n) |
| Multiplication | O(m·n·p) |
| Transpose | O(m·n) |
| Determinant (Gaussian elim.) | O(n³) |
| Inverse (Gauss-Jordan) | O(n³) |

---

## Benchmark Results

### List Operations

#### Array List

| N | Append (µs/op) | Get-seq (µs/op) | Get-rand (µs/op) | Remove-tail (µs/op) |
|---:|---:|---:|---:|---:|
| 100 | 0.019 | 0.003 | 0.018 | 0.011 |
| 1,000 | 0.017 | 0.002 | 0.015 | 0.010 |
| 10,000 | 0.013 | 0.002 | 0.008 | 0.006 |
| 50,000 | 0.012 | 0.002 | 0.009 | 0.006 |

Array list operations are nearly constant time regardless of size. Append is amortized O(1) with geometric growth. Sequential gets run at approximately 2 ns per element because the backing array lives in contiguous memory.

#### Linked List

| N | Append (µs/op) | Get-seq (µs/op) | Get-rand (µs/op) | Remove-tail (µs/op) |
|---:|---:|---:|---:|---:|
| 100 | 0.019 | 0.049 | 0.054 | 0.071 |
| 1,000 | 0.016 | 0.600 | 0.621 | 0.645 |
| 10,000 | 0.025 | 6.377 | 6.337 | 6.346 |
| 50,000 | 0.031 | 31.581 | 31.469 | 31.628 |

Linked list appends remain constant time because of the tail pointer, but index-based access grows linearly. At 50,000 elements random access costs over 31 µs per operation, roughly 1,500× slower than the array list.

#### Sorted List

| N | Append (µs/op) | Get-seq (µs/op) | Get-rand (µs/op) | Remove-tail (µs/op) |
|---:|---:|---:|---:|---:|
| 100 | 0.026 | 0.003 | 0.016 | 0.006 |
| 1,000 | 0.035 | 0.002 | 0.010 | 0.006 |
| 10,000 | 0.030 | 0.002 | 0.010 | 0.006 |
| 50,000 | 0.032 | 0.002 | 0.009 | 0.006 |

Insertion uses binary search to find the correct position, then `memmove` to shift elements. Because the benchmark inserts in sorted order, shifts are minimal and the per-operation cost remains low. Worst-case insertions into the head of a large sorted list will show O(n) behavior from the shift.

#### Array List vs Linked List — Key Comparison

| Metric (N = 10,000) | Array List | Linked List | Ratio |
|---|---:|---:|---:|
| Append | 0.013 µs | 0.025 µs | 1.9× |
| Sequential get | 0.002 µs | 6.377 µs | 3,189× |
| Random get | 0.008 µs | 6.337 µs | 792× |
| Remove from tail | 0.006 µs | 6.346 µs | 1,058× |

Array lists dominate for indexed access patterns. Linked lists are preferable when frequent insertions and removals at known nodes matter more than random access.

---

### Heap Operations

#### Binary Heap

| N | Put (µs/op) | Pop (µs/op) | Peek (µs/op) |
|---:|---:|---:|---:|
| 100 | 0.020 | 0.039 | 0.002 |
| 1,000 | 0.017 | 0.061 | 0.002 |
| 10,000 | 0.017 | 0.093 | 0.002 |
| 50,000 | 0.017 | 0.137 | 0.002 |
| 100,000 | 0.017 | 0.196 | 0.002 |

Put operations run near-constant time because most insertions only sift up a few levels. Pop grows logarithmically as expected, reaching ~0.2 µs at 100K elements.

#### Fibonacci Heap

| N | Put (µs/op) | Pop (µs/op) | Peek (µs/op) |
|---:|---:|---:|---:|
| 100 | 0.022 | 0.226 | 0.002 |
| 1,000 | 0.017 | 0.305 | 0.002 |
| 10,000 | 0.017 | 0.459 | 0.002 |
| 50,000 | 0.030 | 0.636 | 0.002 |
| 100,000 | 0.030 | 0.686 | 0.002 |

Fibonacci heap inserts are O(1) amortized and match binary heap performance. Pop is slower in practice due to consolidation overhead and pointer-chasing through the circular root list.

#### Fibonacci Heap decrease\_key

| N | decrease\_key (µs/op) |
|---:|---:|
| 1,000 | 0.004 |
| 10,000 | 0.003 |
| 50,000 | 0.003 |
| 100,000 | 0.003 |

The decrease_key operation runs in amortized O(1) time through cascading cuts. The consistent ~3 ns per operation confirms the theoretical bound and demonstrates the key advantage of Fibonacci heaps for algorithms like Dijkstra's shortest path.

#### Binary Heap vs Fibonacci Heap — Key Comparison

| Metric (N = 100,000) | Binary Heap | Fibonacci Heap | Winner |
|---|---:|---:|---|
| Put | 0.017 µs | 0.030 µs | Binary |
| Pop | 0.196 µs | 0.686 µs | Binary |
| Peek | 0.002 µs | 0.002 µs | Tie |
| decrease\_key | not supported | 0.003 µs | Fibonacci |

Binary heap is faster for simple insert/pop workloads due to cache-friendly array storage. Fibonacci heap wins when decrease\_key is frequent, such as in Dijkstra's algorithm or minimum spanning tree computation.

---

### Balanced Tree Operations

#### AVL Tree

| N | Insert (µs/op) | Search (µs/op) | Delete (µs/op) | Min (µs/op) |
|---:|---:|---:|---:|---:|
| 100 | 0.092 | 0.025 | 0.078 | 0.006 |
| 1,000 | 0.108 | 0.036 | 0.099 | 0.006 |
| 10,000 | 0.161 | 0.074 | 0.153 | 0.007 |
| 50,000 | 0.257 | 0.152 | 0.247 | 0.009 |
| 100,000 | 0.317 | 0.211 | 0.368 | 0.010 |

#### Red-Black Tree

| N | Insert (µs/op) | Search (µs/op) | Delete (µs/op) | Min (µs/op) |
|---:|---:|---:|---:|---:|
| 100 | 0.086 | 0.026 | 0.071 | 0.006 |
| 1,000 | 0.091 | 0.036 | 0.079 | 0.006 |
| 10,000 | 0.131 | 0.076 | 0.120 | 0.007 |
| 50,000 | 0.207 | 0.157 | 0.193 | 0.010 |
| 100,000 | 0.264 | 0.223 | 0.256 | 0.011 |

#### AVL vs Red-Black — Key Comparison

| Metric (N = 100,000) | AVL | Red-Black | Winner |
|---|---:|---:|---|
| Insert | 0.317 µs | 0.264 µs | Red-Black |
| Search | 0.211 µs | 0.223 µs | AVL |
| Delete | 0.368 µs | 0.256 µs | Red-Black |
| Min | 0.010 µs | 0.011 µs | Tie |

Red-Black trees are faster for mutation-heavy workloads because they perform fewer rotations on average. AVL trees offer marginally faster lookups due to stricter height balance, which can matter in read-heavy scenarios.

---

### Sorting Algorithm Performance

#### Quicksort on Array List

| N | Total (µs) | Per-element (µs/op) | Binary Search (µs/op) |
|---:|---:|---:|---:|
| 100 | 27 | 0.271 | 0.054 |
| 1,000 | 1,596 | 1.596 | 0.078 |
| 5,000 | 40,461 | 8.092 | 0.100 |
| 10,000 | 193,783 | 19.378 | 0.113 |
| 25,000 | 1,562,900 | 62.516 | 0.133 |

The quicksort implementation routes all element access through the abstract list interface, using `get`, `insert`, and `remove` for each swap. This abstraction overhead causes the per-element cost to grow faster than the expected O(n log n) curve. Binary search shows clean O(log n) scaling with sub-microsecond lookups at all sizes.

---

### Matrix Multiplication Scaling

| N | Multiply (ms) | Add (ms) | Transpose (ms) | Determinant (ms) |
|---:|---:|---:|---:|---:|
| 10 | 0.001 | 0.000 | 0.000 | 0.001 |
| 50 | 0.092 | 0.002 | 0.002 | 0.019 |
| 100 | 0.800 | 0.005 | 0.006 | 0.126 |
| 200 | 7.381 | 0.029 | 0.032 | 0.956 |
| 500 | 120.239 | 0.183 | 0.323 | — |

Multiplication follows the expected O(n³) curve. Doubling the matrix dimension from 100 to 200 increases multiplication time by ~9×, consistent with 2³ = 8. Addition and transpose scale as O(n²). Determinant computation via Gaussian elimination also runs in O(n³) but with smaller constant factors than multiplication.

---

### Hash Map Operations

| N | Put (µs/op) | Get (µs/op) | Remove (µs/op) |
|---:|---:|---:|---:|
| 100 | 0.050 | 0.016 | 0.028 |
| 1,000 | 0.046 | 0.009 | 0.021 |
| 10,000 | 0.044 | 0.012 | 0.022 |
| 50,000 | 0.040 | 0.012 | 0.024 |
| 100,000 | 0.067 | 0.013 | 0.025 |

All hash map operations remain effectively constant time across all tested sizes. The slight increase in put time at 100K reflects periodic rehashing when the load factor exceeds 0.75. Get operations stay under 15 ns even at scale.

---

### Set Operations (Hash Set)

| N | Add (µs/op) | Contains (µs/op) | Remove (µs/op) |
|---:|---:|---:|---:|
| 100 | 0.076 | 0.016 | 0.030 |
| 1,000 | 0.061 | 0.012 | 0.024 |
| 10,000 | 0.060 | 0.016 | 0.025 |
| 50,000 | 0.056 | 0.015 | 0.027 |

Hash set performance mirrors the underlying hash map with minimal overhead from the set wrapper layer.

---

### Deque Operations

| N | Push-front (µs/op) | Push-back (µs/op) | Pop-front (µs/op) | Pop-back (µs/op) | Random-get (µs/op) |
|---:|---:|---:|---:|---:|---:|
| 100 | 0.017 | 0.012 | 0.009 | 0.007 | 0.016 |
| 1,000 | 0.007 | 0.006 | 0.009 | 0.007 | 0.009 |
| 10,000 | 0.008 | 0.007 | 0.009 | 0.007 | 0.009 |
| 100,000 | 0.007 | 0.007 | 0.009 | 0.007 | 0.011 |

All deque operations run in constant amortized time. The circular buffer design enables O(1) random access by index, combining the strengths of arrays and double-ended queues. Push operations include geometric resizing cost amortized across insertions.

---

### Stack Operations

| N | Array Push (µs/op) | Array Pop (µs/op) | Linked Push (µs/op) | Linked Pop (µs/op) |
|---:|---:|---:|---:|---:|
| 1,000 | 0.010 | 0.012 | 0.016 | 0.014 |
| 10,000 | 0.008 | 0.012 | 0.015 | 0.014 |
| 100,000 | 0.008 | 0.012 | 0.015 | 0.014 |

Both stack implementations achieve constant-time push and pop. The array stack is slightly faster due to cache locality from contiguous memory, while the linked stack pays a small allocation cost per push.

---

### Queue Operations

| N | Enqueue (µs/op) | Dequeue (µs/op) |
|---:|---:|---:|
| 1,000 | 0.015 | 0.009 |
| 10,000 | 0.015 | 0.008 |
| 100,000 | 0.014 | 0.009 |

FIFO queue operations are constant time at all sizes, running under 15 ns per operation.

---

### Bloom Filter Operations

| N | Add (µs/op) | Contains-hit (µs/op) | Contains-miss (µs/op) |
|---:|---:|---:|---:|
| 1,000 | 0.071 | 0.028 | 0.072 |
| 10,000 | 0.072 | 0.031 | 0.074 |
| 100,000 | 0.071 | 0.032 | 0.077 |

Bloom filter operations are O(k) where k is the number of hash functions, independent of the number of stored elements. Contains-hit checks terminate early when a matching bit pattern is found. Contains-miss queries are slightly slower because they must evaluate all k hash functions before returning.

---

### Trie Operations

| N | Insert (µs/op) | Search-hit (µs/op) | Search-miss (µs/op) | Prefix-check (µs/op) |
|---:|---:|---:|---:|---:|
| 100 | 0.812 | 0.038 | 0.050 | 0.007 |
| 1,000 | 4.026 | 0.123 | 0.047 | 0.010 |
| 10,000 | 6.321 | 0.264 | 0.056 | 0.019 |

Trie insert cost increases with N primarily due to memory allocation pressure (each new node allocates a 26-pointer child array). Search and prefix checks are O(m) where m is the word length, independent of the number of stored words. Prefix checks are particularly fast because they only need to verify existence of a path.

---

### Graph Algorithm Scaling

#### BFS / DFS (Adjacency List, ~2V edges)

| V | Add-edges (µs) | BFS (µs) | DFS-iterative (µs) |
|---:|---:|---:|---:|
| 100 | 9 | 6 | 12 |
| 500 | 39 | 31 | 59 |
| 1,000 | 78 | 62 | 113 |
| 5,000 | 401 | 385 | 278 |

Both BFS and DFS scale linearly with V + E as expected. DFS uses an explicit iterative stack and may have different constant factors than BFS depending on graph structure.

#### Dijkstra (Adjacency List, ~2V edges)

| V | Dijkstra (µs) |
|---:|---:|
| 100 | 13 |
| 500 | 73 |
| 1,000 | 168 |
| 2,000 | 351 |

Dijkstra's algorithm shows near-linear scaling on sparse graphs (E ≈ 2V), consistent with the O((V + E) log V) bound where the log V factor remains small.

---

## Memory Overhead

| Data Structure | Per-element Overhead | Notes |
|---|---:|---|
| Array List | 8 bytes | Contiguous `void*` array; geometric growth may allocate up to 2× required capacity |
| Linked List | 24 bytes | `data` + `next` + `prev` pointers per node |
| Sorted List | 8 bytes | Same layout as array list, maintained in sorted order |
| Binary Heap | 8 bytes | Array-stored `void*`; geometric growth may allocate up to 2× required capacity |
| Fibonacci Heap | ~56 bytes/node | parent, child, left, right pointers + degree, mark, data |
| Hash Map | ~32 bytes/entry | key + data + next pointer per entry; bucket array overhead |
| AVL Tree | ~40 bytes/node | key, value, left, right pointers + height field |
| Red-Black Tree | ~48 bytes/node | key, value, left, right, parent pointers + color flag |
| Trie | ~216 bytes/node | 26 child pointers (208 bytes) + value pointer + end flag |
| Deque | 8 bytes | Circular buffer of `void*`; geometric growth may allocate up to 2× required capacity |
| Bloom Filter | ~1.2 bytes/element | At 1% FPP: 9.6 bits per element with optimal hash count |
| Graph (Adj. List) | ~24 bytes/edge | Edge struct with weight, neighbor ID, next pointer |
| Graph (Adj. Matrix) | 8×V² bytes | Dense `double` matrix regardless of edge count |

### Memory Guidance

- **Array-based structures** (array list, sorted list, binary heap, deque) use 8 bytes per element but may allocate up to twice the needed capacity due to geometric doubling. Actual waste averages around 25% of capacity.
- **Pointer-based structures** (linked list, Fibonacci heap, trees) have higher per-node overhead but never allocate unused capacity. They trade memory efficiency for flexibility in structure manipulation.
- **Hash maps** resize when load factor exceeds 0.75, temporarily doubling memory during rehashing. The bucket array contributes 8 bytes per slot beyond the entries themselves.
- **Tries** are memory-intensive due to the fixed 26-pointer child array per node. For dense prefix spaces this is acceptable; for sparse word sets, a compressed trie would reduce overhead.
- **Bloom filters** are the most memory-efficient structure in the library, using approximately 9.6 bits per element at 1% false positive probability.

---

## Cache Behavior

| Data Structure | Cache Behavior | Explanation |
|---|---|---|
| Array List | Excellent | Contiguous memory enables hardware prefetch; sequential access streams through cache lines |
| Sorted List | Excellent | Same contiguous layout; binary search accesses O(log n) cache lines |
| Binary Heap | Good | Array layout maintains locality for sift-up/down; parent-child pairs are nearby |
| Deque | Good | Circular buffer is contiguous; random access by index hits predictable locations |
| Linked List | Poor | Each node is separately allocated; pointer chasing causes frequent cache misses |
| Fibonacci Heap | Poor | Circular doubly-linked lists scatter nodes across memory; consolidation amplifies misses |
| AVL / Red-Black Tree | Moderate | Nodes allocated individually; tree traversal follows pointers but O(log n) depth limits total misses |
| Hash Map | Moderate | Bucket array is contiguous, but chains follow node pointers; lookup touches 2–3 cache lines on average |
| Trie | Poor | Each node is 216+ bytes with 26 child pointers; descent follows separate allocations |
| Graph (Adj. List) | Moderate | Vertex lookup is array-indexed; neighbor traversal follows linked-list chains |
| Graph (Adj. Matrix) | Good | Dense 2D array has spatial locality for row-wise access; column access is less cache-friendly |

### Cache Miss Impact

The benchmarks demonstrate that cache effects dominate real-world performance for index-based access patterns:

- **Array list random get** at 10K elements: 0.008 µs/op (cache-friendly contiguous access)
- **Linked list random get** at 10K elements: 6.337 µs/op (pointer-chasing cache misses)

This ~792× performance gap far exceeds the theoretical O(1) vs O(n) complexity difference for small n values, illustrating that cache behavior can matter more than algorithmic complexity for moderate data sizes.

Similarly, the binary heap outperforms the Fibonacci heap on pop operations (0.196 vs 0.686 µs/op at 100K) despite both having O(log n) amortized cost, because the array layout of the binary heap produces far fewer cache misses during the sift-down phase.

---

## Recommendations

### When to Use Each Structure

| Use Case | Recommended Structure | Reason |
|---|---|---|
| Indexed collection, frequent reads | Array List | O(1) random access, cache-friendly |
| Frequent insert/remove at ends | Deque | O(1) amortized push/pop at both ends with random access |
| Ordered iteration with fast lookup | Sorted List or Tree Set | Binary search on sorted array, or in-order tree traversal |
| LIFO pattern | Array Stack | Slightly faster than linked stack due to cache locality |
| FIFO pattern | Queue (NORMAL) | O(1) enqueue/dequeue with linked nodes |
| Priority scheduling | Binary Heap / Priority Queue | Simple and fast for insert + extract-min |
| Graph algorithms (Dijkstra, MST) | Fibonacci Heap | O(1) decrease\_key amortizes the per-node pointer overhead |
| Key-value lookup | Hash Map with FNV-1a hash | O(1) average; FNV-1a offers good distribution for general keys |
| Membership testing | Hash Set or Bloom Filter | Hash set for exact membership; bloom filter for probabilistic with minimal memory |
| Autocomplete / prefix search | Trie | O(m) prefix lookups independent of dictionary size |
| Dense graph algorithms | Adjacency Matrix | O(1) edge lookup; Floyd-Warshall benefits from matrix layout |
| Sparse graph algorithms | Adjacency List | Memory-efficient for E ≪ V² graphs |
| Balanced key-value store, read-heavy | AVL Tree | Stricter balance yields faster lookups |
| Balanced key-value store, write-heavy | Red-Black Tree | Fewer rotations on insert and delete |
| Small dense matrix math | Matrix | Native O(n³) multiplication sufficient below N ≈ 500 |
