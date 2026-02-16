# GitHub Issues Draft for ova-lib Recommendations

This document contains draft issues based on the code review recommendations from February 15, 2026.
Each issue is formatted for easy copy-paste into GitHub's issue tracker.

---

## 🚀 NEW FEATURES (10 Issues)

### Issue #1: Implement Graph Data Structure and Algorithms

**Title:** Implement Graph Data Structure and Algorithms

**Labels:** `enhancement`, `feature`, `priority: critical`

**Description:**

Implement a comprehensive graph abstraction supporting both directed and undirected graphs, with adjacency list and adjacency matrix representations.

#### Proposed API
```c
typedef enum {
    GRAPH_DIRECTED,
    GRAPH_UNDIRECTED
} graph_type;

typedef enum {
    GRAPH_ADJACENCY_LIST,
    GRAPH_ADJACENCY_MATRIX
} graph_representation;

graph* create_graph(graph_type type, graph_representation rep);
void graph_add_vertex(graph *g, int vertex_id);
void graph_add_edge(graph *g, int from, int to, double weight);
void graph_remove_edge(graph *g, int from, int to);
bool graph_has_edge(graph *g, int from, int to);
list* graph_get_neighbors(graph *g, int vertex);
```

#### Algorithms to Implement
- **Traversal:** BFS, DFS (iterative and recursive)
- **Shortest Path:** Dijkstra, Bellman-Ford, Floyd-Warshall
- **Minimum Spanning Tree:** Kruskal, Prim
- **Connectivity:** Connected components, strongly connected components (Tarjan/Kosaraju)
- **Topological Sort:** Kahn's algorithm
- **Cycle Detection:** For both directed and undirected graphs

#### Files to Create
- `include/graph.h`
- `src/graph/graph.c`
- `src/graph/graph_algorithms.c`
- `src/graph/adjacency_list.c`
- `src/graph/adjacency_matrix.c`
- `test/test_graph.c`
- `test/test_graph_algorithms.c`
- `docs/graph.md`

#### Dependencies
- Requires Fibonacci heap decrease-key for optimal Dijkstra
- Uses existing queue for BFS
- Uses existing stack for DFS
- Uses existing priority queue for MST algorithms

**Effort:** Large (3-4 weeks)  
**Priority:** ⭐⭐⭐⭐⭐ Critical  
**Impact:** High - Opens entire domain of graph algorithms

---

### Issue #2: Implement Balanced Binary Search Trees (AVL/Red-Black)

**Title:** Implement Balanced Binary Search Trees (AVL/Red-Black)

**Labels:** `enhancement`, `feature`, `priority: critical`

**Description:**

Implement self-balancing binary search trees for ordered data access with guaranteed logarithmic time complexity. Choose AVL for simpler implementation or Red-Black for better insertion performance.

#### Proposed API
```c
typedef enum {
    TREE_AVL,
    TREE_RED_BLACK
} tree_type;

tree* create_tree(tree_type type, comparator cmp);
void tree_insert(tree *t, void *key, void *value);
void* tree_search(tree *t, void *key);
void tree_delete(tree *t, void *key);
void* tree_min(tree *t);
void* tree_max(tree *t);
void* tree_predecessor(tree *t, void *key);
void* tree_successor(tree *t, void *key);
list* tree_range_query(tree *t, void *low, void *high);
void tree_in_order_traverse(tree *t, void (*callback)(void*, void*));
```

#### Why This Matters
- Sorted list uses O(n) insertion - trees provide O(log n)
- Hash maps don't support range queries - trees do
- Enables ordered iteration
- Foundation for more advanced structures (interval trees, segment trees)

#### Implementation Notes
- AVL trees: Simpler rotation logic, height-balanced
- Red-Black trees: More complex but better insertion performance
- Consider implementing both variants

#### Files to Create
- `include/tree.h`
- `src/tree/tree.c`
- `src/tree/avl_tree.c`
- `src/tree/red_black_tree.c`
- `test/test_avl_tree.c`
- `test/test_red_black_tree.c`
- `docs/trees.md`

**Effort:** Medium-Large (2-3 weeks)  
**Priority:** ⭐⭐⭐⭐⭐ Critical  
**Impact:** High - Provides guaranteed O(log n) operations

---

### Issue #3: Complete Fibonacci Heap Implementation

**Title:** Complete Fibonacci Heap Implementation (decrease-key, cascading cuts)

**Labels:** `enhancement`, `feature`, `priority: critical`, `good first issue`

**Description:**

Complete the Fibonacci heap by implementing the missing decrease-key and cascading cut operations. This is essential for optimal Dijkstra's algorithm and other applications.

#### Missing Operations
```c
void fibonacci_heap_decrease_key(heap *h, void *node_handle, void *new_value);
void fibonacci_heap_delete(heap *h, void *node_handle);
```

#### What to Implement
1. **Node handles:** Return opaque references to inserted nodes
2. **Decrease-key:** Reduce node value, cut if heap property violated, cascade up
3. **Cascading cuts:** Mark/cut chain to maintain amortized bounds
4. **Delete operation:** Decrease to -∞, then extract-min

#### Files to Modify
- `include/heap.h` - Add new function signatures
- `src/heap/fibonacci_heap.c` - Implement decrease-key, cascading cuts
- `include/heap_internal.h` - Add mark field to node structure
- `test/test_fibonacci_heap.c` - Add tests for new operations
- `docs/containers.md` - Update documentation to reflect completion

#### Testing
- Verify amortized O(1) decrease-key
- Test cascading cuts trigger correctly
- Validate delete operation
- Run Dijkstra benchmark comparing with binary heap

**Effort:** Small (1 week)  
**Priority:** ⭐⭐⭐⭐⭐ Critical  
**Impact:** Medium - Unlocks advanced graph algorithms

---

### Issue #4: Implement Set Data Structure

**Title:** Implement Set Data Structure

**Labels:** `enhancement`, `feature`, `priority: high`

**Description:**

Implement a set data structure for storing unique elements with fast membership testing, union, intersection, and difference operations.

#### Proposed API
```c
typedef enum {
    SET_HASH,      // Hash-based, O(1) operations
    SET_TREE       // Tree-based, ordered, O(log n) operations
} set_type;

set* create_set(set_type type, comparator cmp, hash_function hash);
bool set_add(set *s, void *element);
bool set_contains(set *s, void *element);
bool set_remove(set *s, void *element);
int set_size(set *s);
set* set_union(set *a, set *b);
set* set_intersection(set *a, set *b);
set* set_difference(set *a, set *b);
bool set_is_subset(set *a, set *b);
list* set_to_list(set *s);
```

#### Implementation Strategies
- **Hash Set:** Wrap hash_map with NULL values
- **Tree Set:** Wrap AVL/RB tree (implement after Issue #2)

#### Use Cases
- Unique element storage
- Membership testing
- Set algebra operations
- Graph vertex/edge sets

#### Files to Create
- `include/set.h`
- `src/set/set.c`
- `src/set/hash_set.c`
- `src/set/tree_set.c` (after tree implementation)
- `test/test_set.c`
- `docs/set.md`

**Effort:** Small (1 week)  
**Priority:** ⭐⭐⭐⭐ High  
**Impact:** Medium - Common use case not currently supported

---

### Issue #5: Implement Deque (Double-Ended Queue)

**Title:** Implement Deque (Double-Ended Queue)

**Labels:** `enhancement`, `feature`, `priority: high`, `good first issue`

**Description:**

Implement a double-ended queue supporting efficient insertion and removal at both ends, plus random access.

#### Proposed API
```c
deque* create_deque(int capacity);
void deque_push_front(deque *d, void *element);
void deque_push_back(deque *d, void *element);
void* deque_pop_front(deque *d);
void* deque_pop_back(deque *d);
void* deque_peek_front(deque *d);
void* deque_peek_back(deque *d);
void* deque_get(deque *d, int index);
int deque_size(deque *d);
bool deque_is_empty(deque *d);
```

#### Implementation
- Circular buffer with dynamic resizing
- O(1) amortized for all push/pop operations
- O(1) random access
- Geometric growth when resizing (2x capacity)

#### Use Cases
- Sliding window algorithms
- BFS/DFS with bidirectional search
- LRU cache implementation
- Work-stealing schedulers

#### Files to Create
- `include/deque.h`
- `src/deque/deque.c`
- `test/test_deque.c`
- `docs/containers.md` - Add deque section

**Effort:** Small (3-5 days)  
**Priority:** ⭐⭐⭐⭐ High  
**Impact:** Medium - Versatile structure for many algorithms

---

### Issue #6: Implement Trie (Prefix Tree)

**Title:** Implement Trie (Prefix Tree)

**Labels:** `enhancement`, `feature`, `priority: medium`

**Description:**

Implement a trie data structure for efficient string storage, prefix matching, and autocomplete functionality.

#### Proposed API
```c
trie* create_trie();
void trie_insert(trie *t, const char *word, void *value);
void* trie_search(trie *t, const char *word);
bool trie_starts_with(trie *t, const char *prefix);
list* trie_get_words_with_prefix(trie *t, const char *prefix);
bool trie_delete(trie *t, const char *word);
int trie_count_words(trie *t);
int trie_count_prefixes(trie *t, const char *prefix);
```

#### Features
- Store arbitrary values associated with strings
- Prefix matching in O(m) where m is string length
- Memory-efficient for common prefixes
- Autocomplete support

#### Implementation Notes
- Node structure with 26-child array (or 256 for extended ASCII)
- Consider compressed trie (radix tree) for space efficiency
- Mark end-of-word nodes

#### Use Cases
- Autocomplete systems
- Spell checkers
- IP routing tables
- Dictionary implementations

#### Files to Create
- `include/trie.h`
- `src/trie/trie.c`
- `test/test_trie.c`
- `docs/trie.md`

**Effort:** Medium (1-2 weeks)  
**Priority:** ⭐⭐⭐ Medium  
**Impact:** Medium - Excellent for string-heavy workloads

---

### Issue #7: Implement Integer Programming Solvers

**Title:** Implement Integer Programming Solvers (Branch-and-Bound, Branch-and-Cut)

**Labels:** `enhancement`, `feature`, `priority: medium`

**Description:**

Implement the stubbed integer programming algorithms: branch-and-bound, branch-and-cut, and Lagrangean relaxation.

#### Current Status
- Enums defined but factories return NULL
- Helper functions exist but unused: `is_feasible()`, `improves_objective()`, `is_integer()`

#### Branch-and-Bound Algorithm
```c
solver* create_solver(SOLVER_BRANCH_AND_BOUND);

// Steps:
1. Solve LP relaxation at root node
2. If solution is integer, done
3. Otherwise, branch on fractional variable
4. Create two subproblems with additional constraints
5. Recursively solve subproblems
6. Track best integer solution found
7. Prune branches worse than incumbent
```

#### Branch-and-Cut
- Extends branch-and-bound with cutting planes
- Tighten LP relaxation before branching
- Common cuts: Gomory, cover inequalities

#### Implementation Steps
1. Implement branch-and-bound first (simpler)
2. Add node priority queue for best-first search
3. Track incumbent solution
4. Implement pruning strategies
5. Add cut generation for branch-and-cut

#### Files to Modify
- `src/solver/solver.c` - Remove stub returns
- `src/solver/branch_and_bound.c` - New file
- `src/solver/branch_and_cut.c` - New file
- `src/solver/lagrangean.c` - New file
- `test/test_solver.c` - Add integer programming tests
- `docs/solver-integer.md` - New documentation

**Effort:** Large (3-4 weeks)  
**Priority:** ⭐⭐⭐ Medium  
**Impact:** Medium - Complete the solver module

---

### Issue #8: Implement Bloom Filter

**Title:** Implement Bloom Filter

**Labels:** `enhancement`, `feature`, `priority: medium`, `good first issue`

**Description:**

Implement a space-efficient probabilistic data structure for set membership testing with tunable false-positive rate.

#### Proposed API
```c
bloom_filter* create_bloom_filter(int expected_elements, double false_positive_rate);
void bloom_filter_add(bloom_filter *bf, const void *element, size_t len);
bool bloom_filter_might_contain(bloom_filter *bf, const void *element, size_t len);
void bloom_filter_clear(bloom_filter *bf);
double bloom_filter_current_fpp(bloom_filter *bf);
```

#### Implementation
- Bit array sized based on expected elements and target FPR
- Multiple hash functions (use FNV-1a with different seeds)
- Calculate optimal k (number of hash functions)
- Formula: m = -n*ln(p) / (ln(2)^2), k = (m/n)*ln(2)

#### Use Cases
- Cache filters (avoid expensive lookups for non-existent keys)
- Spell checking (quickly reject non-words)
- Network routers (packet filtering)
- Distributed systems (membership testing)

#### Files to Create
- `include/bloom_filter.h`
- `src/bloom_filter/bloom_filter.c`
- `test/test_bloom_filter.c`
- `docs/bloom_filter.md`

**Effort:** Small (3-5 days)  
**Priority:** ⭐⭐⭐ Medium  
**Impact:** Low-Medium - Niche but powerful

---

### Issue #9: Implement Skip List

**Title:** Implement Skip List

**Labels:** `enhancement`, `feature`, `priority: low`

**Description:**

Implement a probabilistic alternative to balanced trees with simpler implementation and comparable performance.

#### Proposed API
```c
skip_list* create_skip_list(int max_level, comparator cmp);
void skip_list_insert(skip_list *sl, void *key, void *value);
void* skip_list_search(skip_list *sl, void *key);
void skip_list_delete(skip_list *sl, void *key);
int skip_list_size(skip_list *sl);
```

#### Characteristics
- Expected O(log n) search, insert, delete
- Simpler than AVL/RB trees (no rotations)
- Probabilistic balancing via random tower heights
- Typical max_level = log(n)

#### Why Skip Lists
- Simpler implementation than balanced trees
- Lock-free variants easier than trees
- Good cache locality with multiple levels
- Educational value

#### Files to Create
- `include/skip_list.h`
- `src/skip_list/skip_list.c`
- `test/test_skip_list.c`
- `docs/skip_list.md`

**Effort:** Medium (1-2 weeks)  
**Priority:** ⭐⭐ Low-Medium  
**Impact:** Low - Alternative to trees

---

### Issue #10: Implement Memory Pool Allocator

**Title:** Implement Memory Pool Allocator

**Labels:** `enhancement`, `feature`, `priority: low`, `performance`

**Description:**

Implement custom memory pool allocators for frequently allocated structures to reduce malloc/free overhead and improve cache locality.

#### Proposed API
```c
typedef struct memory_pool memory_pool;

memory_pool* create_memory_pool(size_t block_size, int initial_blocks);
void* memory_pool_alloc(memory_pool *pool);
void memory_pool_free(memory_pool *pool, void *ptr);
void memory_pool_destroy(memory_pool *pool);
```

#### Strategy
- Pre-allocate large chunks of memory
- Maintain free list for recycling
- O(1) allocation/deallocation
- Thread-local pools for concurrent access

#### Apply to
- Linked list nodes (frequent alloc/free)
- Tree nodes (AVL, RB, skip list)
- Hash table buckets
- Graph edge/vertex objects

#### Benefits
- Reduced malloc overhead
- Better cache locality (contiguous allocation)
- Predictable memory usage
- Faster allocation/deallocation

#### Files to Create
- `include/memory_pool.h`
- `src/memory_pool/memory_pool.c`
- `test/test_memory_pool.c`
- Integrate into existing structures

#### Benchmark
- Compare linked list performance with/without pool
- Measure allocation time reduction
- Profile cache miss rates

**Effort:** Medium (1-2 weeks)  
**Priority:** ⭐⭐ Low-Medium  
**Impact:** Medium - Performance improvement

---

## 🐛 CRITICAL BUG FIXES (5 Issues)

### Issue #11: Fix Realloc Failure Handling in Array List

**Title:** [BUG] Fix realloc failure handling in array_list.c

**Labels:** `bug`, `priority: critical`, `good first issue`

**Description:**

#### Problem
In `src/list/array_list.c` line ~54 in `ensure_capacity()`, if `realloc()` fails and returns NULL, `impl->items` becomes NULL but code continues execution, leading to NULL pointer corruption.

#### Current Code (UNSAFE)
```c
impl->items = realloc(impl->items, new_capacity * sizeof(void *));
impl->capacity = new_capacity;
```

#### Fixed Code (SAFE)
```c
void **new_items = realloc(impl->items, new_capacity * sizeof(void *));
if (new_items == NULL) {
    return false; // Signal failure
}
impl->items = new_items;
impl->capacity = new_capacity;
return true;
```

#### Files to Fix
- `src/list/array_list.c` - `ensure_capacity()`
- `src/matrix/matrix.c` - `matrix_resize()`, `vector_resize()`
- `src/stack/array_stack.c` - capacity expansion

#### Testing
- Test allocation failure paths
- Ensure state remains valid after failed realloc
- Add unit tests for out-of-memory scenarios

**Priority:** ⭐⭐⭐⭐⭐ CRITICAL - Can cause data corruption  
**Effort:** Small (1-2 days)

---

### Issue #12: Add Integer Overflow Protection for Capacity Doubling

**Title:** [BUG] Add integer overflow protection for capacity doubling

**Labels:** `bug`, `priority: critical`

**Description:**

#### Problem
In multiple files, `capacity * 2` can overflow for large capacities, leading to undefined behavior or incorrect allocation sizes.

#### Solution
Add a safe capacity doubling function:

```c
static int safe_double_capacity(int current_capacity) {
    if (current_capacity > INT_MAX / 2) {
        return INT_MAX; // Maximum safe capacity
    }
    return current_capacity * 2;
}
```

#### Files to Fix
- `src/list/array_list.c` - Array list growth
- `src/map/hash_map.c` - Hash map resizing
- `src/stack/array_stack.c` - Array stack expansion
- Any other dynamic array implementations

#### Testing
- Test with large capacities near INT_MAX/2
- Verify overflow protection works correctly
- Ensure graceful handling when max capacity reached

**Priority:** ⭐⭐⭐⭐⭐ CRITICAL - Can cause buffer overflows  
**Effort:** Small (1 day)

---

### Issue #13: Validate Mutex Initialization in Hash Map

**Title:** [BUG] Validate pthread_mutex_init return value in hash_map.c

**Labels:** `bug`, `priority: critical`, `concurrency`

**Description:**

#### Problem
In `src/map/hash_map.c` line ~33, `pthread_mutex_init()` can fail but the return value is not checked, potentially leading to use of uninitialized mutex.

#### Current Code (UNSAFE)
```c
if (use_mutex) {
    impl->mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(impl->mutex, NULL); // Return value not checked
}
```

#### Fixed Code (SAFE)
```c
if (use_mutex) {
    impl->mutex = malloc(sizeof(pthread_mutex_t));
    if (impl->mutex == NULL) {
        // cleanup and return NULL
    }
    if (pthread_mutex_init(impl->mutex, NULL) != 0) {
        free(impl->mutex);
        // cleanup and return NULL
    }
}
```

#### Testing
- Test mutex initialization failure scenarios
- Verify proper cleanup on failure
- Add concurrent access tests

**Priority:** ⭐⭐⭐⭐⭐ CRITICAL - Can cause concurrency issues  
**Effort:** Small (1 day)

---

### Issue #14: Add Bounds Checking in Matrix Operations

**Title:** [BUG] Add dimension validation in matrix operations

**Labels:** `bug`, `priority: critical`

**Description:**

#### Problem
Some matrix operations in `src/matrix/matrix.c` don't validate that dimensions match before performing operations, leading to potential crashes or incorrect results.

#### Solution
Add dimension checks to all matrix operations:

```c
matrix* matrix_add(matrix *a, matrix *b) {
    if (a == NULL || b == NULL) return NULL;
    if (a->rows != b->rows || a->cols != b->cols) {
        fprintf(stderr, "Matrix dimension mismatch\n");
        return NULL;
    }
    // proceed with addition
}
```

#### Functions to Fix
- `matrix_add()` - verify dimensions match
- `matrix_subtract()` - verify dimensions match
- `matrix_multiply()` - verify a->cols == b->rows

#### Testing
- Test with mismatched dimensions
- Verify error messages are clear
- Ensure NULL is returned safely

**Priority:** ⭐⭐⭐⭐⭐ CRITICAL - Can cause crashes  
**Effort:** Small (1 day)

---

### Issue #15: Implement Safe String Comparison in Hash Map

**Title:** [BUG] Add NULL-safe key comparison in hash_map.c

**Labels:** `bug`, `priority: high`

**Description:**

#### Problem
NULL key comparisons in hash map might segfault if the comparator doesn't handle NULL values.

#### Solution
Add a safety wrapper:

```c
static bool safe_key_equals(void *k1, void *k2, key_comparator cmp) {
    if (k1 == NULL && k2 == NULL) return true;
    if (k1 == NULL || k2 == NULL) return false;
    return cmp(k1, k2);
}
```

Use this wrapper instead of calling the comparator directly.

#### Testing
- Test with NULL keys
- Verify no segfaults occur
- Add unit tests for edge cases

**Priority:** ⭐⭐⭐⭐ HIGH - Can cause crashes  
**Effort:** Small (1 day)

---

## 🔧 API IMPROVEMENTS (5 Issues)

### Issue #16: Add Consistent Error Code Return Values

**Title:** Add consistent error code enum for all operations

**Labels:** `enhancement`, `api-improvement`, `priority: high`

**Description:**

#### Problem
Currently there's a mix of NULL returns and silent failures, making it hard to distinguish between different failure modes.

#### Solution
Add a consistent error code enum to `include/types.h`:

```c
typedef enum {
    OVA_SUCCESS = 0,
    OVA_ERROR_MEMORY = -1,
    OVA_ERROR_INVALID_ARG = -2,
    OVA_ERROR_INDEX_OUT_OF_BOUNDS = -3,
    OVA_ERROR_EMPTY = -4,
    OVA_ERROR_FULL = -5,
    OVA_ERROR_NOT_FOUND = -6
} ova_error_code;
```

#### Apply To
- All insertion operations
- All deletion operations
- Size changes (resize, expand)

#### Benefits
- Distinguish between different failure modes
- Easier debugging
- Better error handling by users

**Priority:** ⭐⭐⭐⭐ HIGH  
**Effort:** Medium (1-2 weeks)

---

### Issue #17: Implement Generic Iterator Interface

**Title:** Implement generic iterator interface for all containers

**Labels:** `enhancement`, `api-improvement`, `priority: high`

**Description:**

#### Problem
Currently there's no standard iteration mechanism across different container types.

#### Solution
Add a generic iterator pattern to `include/types.h`:

```c
typedef struct iterator {
    void *collection;
    void *impl; // Iterator state
    bool (*has_next)(struct iterator*);
    void* (*next)(struct iterator*);
    void (*remove)(struct iterator*); // Optional
    void (*free)(struct iterator*);
} iterator;
```

#### Implement For
- Lists (array, linked, sorted)
- Sets
- Maps (key iterator, value iterator, entry iterator)
- Trees (in-order, pre-order, post-order)

#### Benefits
- Uniform traversal interface
- Safer than exposing internal structure
- Supports concurrent modification detection

**Priority:** ⭐⭐⭐⭐ HIGH  
**Effort:** Medium (1-2 weeks)

---

### Issue #18: Add Shallow and Deep Copy Operations

**Title:** Add clone operations (shallow and deep copy) to all containers

**Labels:** `enhancement`, `api-improvement`, `priority: medium`

**Description:**

#### Problem
Currently no copy operations exist for any container types.

#### Solution
Add clone methods with explicit copy semantics:

```c
// Shallow copy - copies structure but not elements
list* list_clone_shallow(list *l);

// Deep copy - requires user-provided element copier
typedef void* (*element_copier)(void*);
list* list_clone_deep(list *l, element_copier copier);
```

#### Apply To
- All container types (list, queue, stack, map, etc.)
- Matrices (already has a copy function but document semantics)

#### Benefits
- Enable container duplication
- Support defensive copying
- Clear semantics (shallow vs deep)

**Priority:** ⭐⭐⭐ MEDIUM  
**Effort:** Medium (1-2 weeks)

---

### Issue #19: Add Clear Operation to All Containers

**Title:** Add clear() operation to all containers

**Labels:** `enhancement`, `api-improvement`, `priority: medium`, `good first issue`

**Description:**

#### Problem
Currently only `destroy()` exists, which deallocates the entire container. There's no way to clear contents while keeping the structure.

#### Solution
Add clear operations:

```c
void list_clear(list *l);
void queue_clear(queue *q);
void stack_clear(stack *s);
void heap_clear(heap *h);
void map_clear(map *m);
```

#### Benefits
- Reuse container without reallocating
- Preserve capacity settings
- More efficient than destroy + create

**Priority:** ⭐⭐⭐ MEDIUM  
**Effort:** Small (1-2 days)

---

### Issue #20: Add User Data Field to All Structures

**Title:** Add user_data field to all container structures

**Labels:** `enhancement`, `api-improvement`, `priority: low`

**Description:**

#### Problem
There's currently no way to attach user context or metadata to containers.

#### Solution
Add a `void* user_data` field to all container structures:

```c
typedef struct list {
    // existing fields
    void *user_data; // User-provided context
} list;
```

#### Use Cases
- Attach metadata
- Pass context to callbacks
- Debugging/profiling hooks

**Priority:** ⭐⭐ LOW  
**Effort:** Small (1 day)

---

## ⚡ PERFORMANCE OPTIMIZATIONS (5 Issues)

### Issue #21: Add Bulk Insert Operations

**Title:** Add bulk insert operations for better performance

**Labels:** `enhancement`, `performance`, `priority: medium`

**Description:**

#### Problem
Inserting multiple elements one-by-one incurs overhead from multiple capacity checks and resizes.

#### Solution
Add bulk insert operations:

```c
void list_insert_bulk(list *l, void **elements, int count);
void set_add_bulk(set *s, void **elements, int count);
void map_put_bulk(map *m, void **keys, void **values, int count);
```

#### Benefits
- Single capacity check and resize
- Better cache locality
- Reduced overhead
- Significant speedup for batch operations

**Priority:** ⭐⭐⭐ MEDIUM  
**Effort:** Small (3-5 days)

---

### Issue #22: Optimize Matrix Multiplication with Strassen's Algorithm

**Title:** Add Strassen's algorithm for large matrix multiplication

**Labels:** `enhancement`, `performance`, `priority: low`

**Description:**

#### Problem
Current matrix multiplication uses naive triple loop O(n³) algorithm.

#### Solution
Add Strassen's algorithm for large matrices:

```c
matrix* matrix_multiply_strassen(matrix *a, matrix *b);
```

#### Implementation
- Use Strassen only for n > 64 (threshold tunable)
- Fall back to naive for small matrices
- Achieves O(n^2.807) complexity

#### Benefits
- Significant speedup for large matrices
- Optional - doesn't affect existing code

**Priority:** ⭐⭐ LOW  
**Effort:** Medium (1 week)

---

### Issue #23: Implement Cache-Oblivious Sorting

**Title:** Add cache-oblivious merge sort variant

**Labels:** `enhancement`, `performance`, `priority: low`

**Description:**

#### Problem
Current quicksort may have poor cache behavior on certain input patterns.

#### Solution
Add cache-oblivious merge sort as an alternative sorting algorithm.

#### Benefits
- Better cache utilization
- More predictable performance
- No tuning parameters needed
- Guaranteed O(n log n) worst case

**Priority:** ⭐⭐ LOW  
**Effort:** Medium (1 week)

---

### Issue #24: Add SIMD Operations for Vector Math

**Title:** Add SIMD-accelerated vector operations

**Labels:** `enhancement`, `performance`, `priority: low`

**Description:**

#### Problem
Vector operations currently process elements one-by-one, not utilizing SIMD instructions.

#### Solution
Use SSE/AVX for vector operations when available:

```c
#ifdef __AVX__
    vector* vector_add_simd(vector *a, vector *b);
    double vector_dot_product_simd(vector *a, vector *b);
    vector* vector_scale_simd(vector *v, double scalar);
#endif
```

#### Operations to Accelerate
- Vector addition/subtraction
- Dot product
- Scalar multiplication
- Element-wise operations

#### Benefits
- 2-8x speedup for large vectors
- Better utilization of modern CPUs
- Optional - fallback to scalar code

**Priority:** ⭐⭐ LOW  
**Effort:** Medium (1-2 weeks)

---

### Issue #25: Implement Small String Optimization in Trie

**Title:** Add small string optimization to trie implementation

**Labels:** `enhancement`, `performance`, `priority: low`

**Description:**

#### Problem
Short strings stored in trie require pointer chasing through node structures.

#### Solution
Store short strings (< 16 bytes) inline in trie nodes instead of through linked nodes.

#### Benefits
- Reduce pointer chasing for common short keys
- Better cache locality
- Faster lookup for short strings
- Common pattern in production string containers

**Note:** This should be implemented when Trie is added (Issue #6)

**Priority:** ⭐⭐ LOW  
**Effort:** Small (3-5 days)

---

## 🧪 TESTING & QA (5 Issues)

### Issue #26: Add AddressSanitizer Integration

**Title:** Add AddressSanitizer build option to CMake

**Labels:** `testing`, `tooling`, `priority: high`, `good first issue`

**Description:**

#### Problem
No automated memory error detection in build system.

#### Solution
Add AddressSanitizer option to `CMakeLists.txt`:

```cmake
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()
```

#### Usage
```bash
cmake -DENABLE_ASAN=ON ..
make
ctest
```

#### Benefits
- Detect buffer overflows
- Detect use-after-free
- Detect memory leaks
- Minimal overhead

**Priority:** ⭐⭐⭐⭐ HIGH  
**Effort:** Small (1 hour)

---

### Issue #27: Add Valgrind Memcheck Integration

**Title:** Add Valgrind memcheck target to CMake

**Labels:** `testing`, `tooling`, `priority: high`, `good first issue`

**Description:**

#### Problem
No automated memory leak detection integrated into build system.

#### Solution
Add Valgrind integration to `CMakeLists.txt`:

```cmake
find_program(VALGRIND_PROGRAM valgrind)
if(VALGRIND_PROGRAM)
    add_custom_target(memcheck
        COMMAND ${CMAKE_CTEST_COMMAND} 
        --force-new-ctest-process --test-action memcheck
        COMMAND cat "${CMAKE_BINARY_DIR}/Testing/Temporary/MemoryChecker.*.log"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
```

#### Usage
```bash
make memcheck
```

#### Benefits
- Detect memory leaks
- Detect invalid memory access
- Detailed error reports
- Industry standard tool

**Priority:** ⭐⭐⭐⭐ HIGH  
**Effort:** Small (1 hour)

---

### Issue #28: Add Code Coverage Reporting

**Title:** Add code coverage reporting to build system

**Labels:** `testing`, `tooling`, `priority: high`

**Description:**

#### Problem
No visibility into test coverage metrics.

#### Solution
Add coverage option to `CMakeLists.txt`:

```cmake
option(ENABLE_COVERAGE "Enable code coverage" OFF)
if(ENABLE_COVERAGE)
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()
```

#### Usage
```bash
cmake -DENABLE_COVERAGE=ON ..
make
ctest
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

#### Target
- Achieve 90%+ line coverage
- Track coverage over time
- Identify untested code paths

**Priority:** ⭐⭐⭐⭐ HIGH  
**Effort:** Small (2-3 hours)

---

### Issue #29: Add Property-Based Testing Framework

**Title:** Add property-based testing for data structures

**Labels:** `testing`, `enhancement`, `priority: medium`

**Description:**

#### Problem
Current unit tests only cover specific scenarios, may miss edge cases.

#### Solution
Add property-based testing framework to test invariants:

#### Properties to Test
- **Lists:** Insertion then retrieval returns same element
- **Sorting:** Result is non-decreasing sequence
- **Sets:** Union is commutative and associative
- **Matrix:** Multiplication is associative

#### Approach
- Use or create simple property-based test framework
- Generate random inputs
- Verify invariants hold

#### Benefits
- Find edge cases automatically
- Test invariants rather than specific scenarios
- Better confidence in correctness

**Priority:** ⭐⭐⭐ MEDIUM  
**Effort:** Medium (1-2 weeks)

---

### Issue #30: Add Fuzzing Support

**Title:** Add fuzzing support for finding bugs

**Labels:** `testing`, `security`, `priority: medium`

**Description:**

#### Problem
No automated fuzzing to find bugs from unexpected inputs.

#### Solution
Create `fuzz/` directory with fuzzing harnesses using AFL++ or libFuzzer.

#### Target Operations
- Hash map with random keys
- Sorting with random inputs
- Matrix operations with random dimensions
- Parser operations (if any)

#### Setup
```bash
# Using libFuzzer
clang -fsanitize=fuzzer,address -o fuzz_hashmap fuzz/fuzz_hashmap.c
./fuzz_hashmap
```

#### Benefits
- Find crashes and hangs
- Discover edge cases
- Improve robustness
- Security hardening

**Priority:** ⭐⭐⭐ MEDIUM  
**Effort:** Medium (1 week)

---

## 📚 DOCUMENTATION (5 Issues)

### Issue #31: Generate API Documentation with Doxygen

**Title:** Add Doxygen documentation for all public APIs

**Labels:** `documentation`, `priority: high`

**Description:**

#### Problem
No auto-generated API documentation exists.

#### Solution
1. Create `Doxyfile` configuration
2. Add Doxygen comments to all public functions
3. Generate HTML documentation

#### Example Doxygen Comment
```c
/**
 * @brief Creates a new list of the specified type
 * @param type The type of list to create
 * @param capacity Initial capacity (ignored for linked lists)
 * @param cmp Comparator function (required for sorted lists)
 * @return Pointer to new list, or NULL on allocation failure
 * @note Caller is responsible for calling free() on returned list
 */
list* create_list(list_type type, int capacity, comparator cmp);
```

#### Generate Documentation
```bash
doxygen Doxyfile
```

#### Target
- 100% of public functions documented
- Include examples where helpful
- Cross-reference related functions

**Priority:** ⭐⭐⭐⭐ HIGH  
**Effort:** Medium (1-2 weeks)

---

### Issue #32: Create Performance Benchmark Documentation

**Title:** Document performance benchmarks for all operations

**Labels:** `documentation`, `performance`, `priority: medium`

**Description:**

#### Problem
No documented performance characteristics or comparisons.

#### Solution
Create `docs/benchmarks.md` with:

#### Include
- Time complexity benchmarks for each operation
- Comparison graphs (array vs linked list, binary vs Fibonacci heap)
- Memory overhead measurements
- Cache miss statistics
- Real-world benchmark results

#### Benchmarks to Run
- List operations at various sizes
- Sorting algorithm comparisons
- Heap operation comparisons
- Matrix multiplication scaling

**Priority:** ⭐⭐⭐ MEDIUM  
**Effort:** Medium (1 week)

---

### Issue #33: Create Usage Best Practices Guide

**Title:** Create comprehensive best practices guide

**Labels:** `documentation`, `priority: medium`, `good first issue`

**Description:**

#### Problem
No guidance on how to use the library effectively.

#### Solution
Create `docs/best-practices.md` covering:

#### Topics
- When to use which data structure
- Memory management guidelines
- Thread-safety considerations
- Error handling patterns
- Performance tips
- Common pitfalls to avoid

#### Examples
- "Use ArrayList for random access, LinkedList for frequent insertions"
- "Always check return values from create_* functions"
- "Use hash maps with mutex for concurrent access"

**Priority:** ⭐⭐⭐ MEDIUM  
**Effort:** Small (3-5 days)

---

### Issue #34: Add Architecture Diagrams

**Title:** Add visual architecture diagrams to documentation

**Labels:** `documentation`, `priority: low`

**Description:**

#### Problem
Complex relationships are hard to understand from text alone.

#### Solution
Create diagrams using PlantUML or Mermaid for:

#### Diagrams Needed
- Factory pattern relationships
- Module dependencies
- Memory layout of structures
- Algorithm flowcharts
- Inheritance/composition relationships

#### Tools
- PlantUML for UML diagrams
- Mermaid for flowcharts
- Graphviz for dependency graphs

**Priority:** ⭐⭐ LOW  
**Effort:** Medium (1 week)

---

### Issue #35: Create CONTRIBUTING.md

**Title:** Create comprehensive CONTRIBUTING.md guide

**Labels:** `documentation`, `priority: medium`, `good first issue`

**Description:**

#### Problem
No guidance for contributors on how to contribute to the project.

#### Solution
Create `CONTRIBUTING.md` with:

#### Include
- Code style guidelines (formatting, naming conventions)
- How to add new data structures (follow factory pattern)
- Test requirements (unit tests mandatory)
- PR process (review requirements, CI must pass)
- Build instructions for developers
- How to report bugs
- How to request features

#### Benefits
- Lower barrier to contribution
- Consistent code quality
- Clear expectations
- Faster PR reviews

**Priority:** ⭐⭐⭐ MEDIUM  
**Effort:** Small (1-2 days)

---

## 🔨 BUILD & TOOLING (5 Issues)

### Issue #36: Add Strict Compiler Warnings

**Title:** Enable strict compiler warnings in CMake

**Labels:** `tooling`, `quality`, `priority: high`

**Description:**

#### Problem
Currently building without strict warnings, potentially hiding bugs.

#### Solution
Add to `CMakeLists.txt`:

```cmake
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wcast-align -Wcast-qual
        -Wconversion -Wsign-conversion
        -Wdouble-promotion
        -Wformat=2 -Wformat-security
        -Wnull-dereference
        -Wshadow
        -Wstrict-overflow=2
        -Wswitch-default -Wswitch-enum
        -Wundef
        -Wuninitialized
        -Wwrite-strings
    )
endif()
```

#### Implementation
- Enable warnings gradually
- Fix all warnings before merging
- Treat warnings as errors in CI

**Priority:** ⭐⭐⭐⭐ HIGH  
**Effort:** Medium (1-2 weeks)

---

### Issue #37: Add Static Analysis Integration

**Title:** Integrate static analysis tools into build/CI

**Labels:** `tooling`, `quality`, `priority: high`

**Description:**

#### Problem
No automated static analysis to catch bugs early.

#### Solution
Integrate multiple static analysis tools:

#### Tools
- **cppcheck:** `cppcheck --enable=all src/`
- **clang-tidy:** Configure `.clang-tidy` file
- **scan-build:** Clang static analyzer

#### CI Integration
```yaml
# .github/workflows/static-analysis.yml
- name: Run cppcheck
  run: cppcheck --error-exitcode=1 --enable=all src/
```

#### Benefits
- Find bugs before runtime
- Enforce coding standards
- Catch common mistakes
- No runtime overhead

**Priority:** ⭐⭐⭐⭐ HIGH  
**Effort:** Small (1 day)

---

### Issue #38: Add Continuous Integration Pipeline

**Title:** Create comprehensive GitHub Actions CI pipeline

**Labels:** `tooling`, `ci`, `priority: high`

**Description:**

#### Problem
No automated testing on different platforms.

#### Solution
Create `.github/workflows/ci.yml` with:

#### Jobs
- Build on Ubuntu, macOS, Windows (MinGW)
- Run full test suite
- Check code coverage (fail if below threshold)
- Run static analyzers
- Run sanitizers (ASan, UBSan, TSan)
- Run Valgrind memcheck
- Build documentation

#### Benefits
- Catch bugs early
- Test on multiple platforms
- Automated quality checks
- Prevent regressions

**Priority:** ⭐⭐⭐⭐ HIGH  
**Effort:** Medium (1 week)

---

### Issue #39: Add CMake Presets

**Title:** Add CMake presets for common build configurations

**Labels:** `tooling`, `developer-experience`, `priority: low`, `good first issue`

**Description:**

#### Problem
Developers need to remember complex CMake command lines.

#### Solution
Create `CMakePresets.json`:

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "dev",
      "displayName": "Development Build",
      "binaryDir": "${sourceDir}/build/dev",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "ENABLE_ASAN": "ON"
      }
    },
    {
      "name": "release",
      "displayName": "Release Build",
      "binaryDir": "${sourceDir}/build/release",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release"
      }
    }
  ]
}
```

#### Usage
```bash
cmake --preset dev
cmake --preset release
```

**Priority:** ⭐⭐ LOW  
**Effort:** Small (1 hour)

---

### Issue #40: Add Package Manager Support

**Title:** Add support for popular C/C++ package managers

**Labels:** `tooling`, `distribution`, `priority: medium`

**Description:**

#### Problem
No easy way for users to integrate library via package managers.

#### Solution
Add support for multiple package managers:

#### Files to Create
- `conanfile.py` for Conan
- `vcpkg.json` for vcpkg
- `ova-lib.pc.in` for pkg-config

#### Example vcpkg.json
```json
{
  "name": "ova-lib",
  "version-string": "0.0.1",
  "description": "Lightweight data structures and algorithms in C",
  "dependencies": []
}
```

#### Benefits
- Easy integration for users
- Standard distribution method
- Version management
- Dependency resolution

**Priority:** ⭐⭐⭐ MEDIUM  
**Effort:** Medium (1 week)

---

## Summary

**Total Issues: 40**
- 🚀 New Features: 10
- 🐛 Critical Bugs: 5
- 🔧 API Improvements: 5
- ⚡ Performance: 5
- 🧪 Testing: 5
- 📚 Documentation: 5
- 🔨 Tooling: 5

### Priority Distribution
- ⭐⭐⭐⭐⭐ Critical: 8 issues
- ⭐⭐⭐⭐ High: 15 issues
- ⭐⭐⭐ Medium: 12 issues
- ⭐⭐ Low: 5 issues

### Estimated Total Effort
- Quick wins (< 1 week): 15 issues
- Medium effort (1-2 weeks): 15 issues
- Large effort (3-4 weeks): 10 issues
- **Total: 6-12 months for complete implementation**

### Recommended Starting Order
1. Critical bug fixes (#11-15) - 1 week
2. Testing infrastructure (#26-28) - 1 week
3. Complete Fibonacci heap (#3) - 1 week
4. Deque (#5) and Set (#4) - 2 weeks
5. AVL Trees (#2) - 3 weeks
6. Graph algorithms (#1) - 4 weeks
7. All other improvements as resources allow

---

**Generated from code review recommendations** | February 15, 2026
