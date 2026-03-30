# Usage Guide

The library is small enough that the choice of container usually matters more than micro-optimizing one function call. A good usage pattern starts with the access pattern, then checks ownership, then checks failure paths.

## Pick the Shape That Matches the Access Pattern

The table below is the shortest path through the current API.

| Need | API | Why |
| --- | --- | --- |
| Indexed reads | `create_list(ARRAY_LIST, ...)` | `get` is constant time |
| Sorted indexed reads | `create_list(SORTED_LIST, ..., cmp)` | order is maintained on insert |
| Many middle inserts with traversal cost accepted | `create_list(LINKED_LIST, ...)` | no array shifting |
| FIFO flow | `create_queue(QUEUE_TYPE_NORMAL, ...)` | enqueue and dequeue are constant time |
| Priority flow | `create_queue(QUEUE_TYPE_PRIORITY, ..., cmp)` | wraps a binary heap |
| Push and pop at both ends | `create_deque(...)` | circular buffer with growth |
| Ordered key lookup and range scans | `create_tree(...)` | tree operations stay logarithmic |
| Unordered key lookup | `create_map(HASH_MAP, ..., hash, cmp)` | average constant-time lookup |
| Shared hash map across threads | `create_map(HASH_TABLE, ..., hash, cmp)` | adds one mutex |
| Prefix lookup | `create_trie()` | cost scales with word length |
| Exact set semantics | `create_set(...)` | wraps map or tree |
| Approximate membership | `create_bloom_filter(n, p)` | bounded false positives |
| Dense graph edge queries | `create_graph(..., GRAPH_ADJACENCY_MATRIX)` | matrix lookup stays constant time |
| Sparse graph traversals | `create_graph(..., GRAPH_ADJACENCY_LIST)` | storage scales with edges |

## Keep Ownership Simple

The library frees internal allocations and leaves payload pointers alone. If you insert `int *`, `char *`, or application structs into a container, the container will not free them for you. The same rule applies to `map` keys and values, `tree` keys and values, `set` elements, and `trie` values.

The practical pattern is short: destroy the container, and free payloads in your own code when they are truly owned by your application.

## Check Constructors and Boundary Cases

Every factory can return `NULL`. That includes `create_list`, `create_queue`, `create_heap`, `create_map`, `create_graph`, `create_matrix`, `create_solver`, `create_set`, `create_trie`, and `create_bloom_filter`.

Boundary behavior is stable enough to rely on if you read it once.

`list->get` returns `NULL` for an invalid index. Invalid list insertions are no-ops in the shipped tests. `queue->dequeue`, `heap->pop`, `deque_pop_front`, and `deque_pop_back` return `NULL` on empty structures. `create_matrix` returns `NULL` for non-positive dimensions. `graph_add_edge` adds missing vertices implicitly when vertex ids are non-negative. `graph_get_neighbors` returns an allocated list even when the vertex is missing.

## Treat Comparators and Hash Functions as Part of the API

The container factories store function pointers and reuse them later. A bad comparator or a mismatched hash function does not fail fast in every case, so the safe rule is to treat those callbacks as part of the data model.

For `SORTED_LIST`, `QUEUE_TYPE_PRIORITY`, both heap types, both tree types, and `SET_TREE`, the comparator defines ordering. For `create_map` and `SET_HASH`, the comparator defines equality and the hash function must agree with that equality rule. For set algebra, both sets must use the same implementation family and the same function pointers; otherwise `set_union`, `set_intersection`, and related helpers return `NULL` or `false`.

## Know the Concurrency Boundary

Only `HASH_TABLE` adds internal locking. Everything else requires external synchronization if more than one thread mutates or reads shared state concurrently.

That rule includes `list`, `queue`, `heap`, `deque`, `tree`, `set`, `trie`, `graph`, `matrix`, and the solver types. The absence of iterators reduces exposure, but it does not make concurrent access safe.

## Use the Docs in Order

Start with [README.md](../README.md) for build and test commands. Move to [containers.md](containers.md) or the module page you need. End with [recommendations.md](recommendations.md) if you want the March 2026 review findings and remaining gaps.
