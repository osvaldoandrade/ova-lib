# Performance Notes

The repo currently ships no benchmark harness under `test/` or `docs/`. That means this page should not claim measured microsecond numbers that cannot be reproduced from the tree. Its job is narrower: describe the performance model implied by the current implementations and show how to benchmark the library locally when timing matters.

## Complexity Model

The table below follows the code in `src/` rather than historical notes.

| Area | Fast path | Slow path |
| --- | --- | --- |
| `ARRAY_LIST` | append and indexed read | middle insert or remove |
| `LINKED_LIST` | head or tail edits | indexed traversal |
| `SORTED_LIST` | indexed read | insert and remove because of shifting |
| `QUEUE_TYPE_NORMAL` | enqueue and dequeue | none beyond allocation |
| `QUEUE_TYPE_PRIORITY` | `peek` and `size` | enqueue and dequeue through heap reordering |
| `BINARY_HEAP` | `peek`, `size` | `put`, `pop` |
| `FIBONACCI_HEAP` | `put`, `peek`, `size` | `pop`, `delete_node` |
| `HASH_MAP` and `HASH_TABLE` | average `put`, `get`, `remove` | collision-heavy buckets or rehash |
| `SET_HASH` | average membership checks | collision-heavy buckets |
| `SET_TREE`, `TREE_AVL`, `TREE_RED_BLACK` | ordered lookup | none beyond logarithmic rebalancing |
| `TRIE` | short-prefix lookup | long-word traversal and prefix materialization |
| `GRAPH_ADJACENCY_LIST` | sparse traversals | dense edge scans |
| `GRAPH_ADJACENCY_MATRIX` | direct edge checks | large sparse graphs |
| `matrix` | small arithmetic kernels | cubic determinant and inverse |

## Cost Drivers in This Codebase

Three implementation choices dominate runtime.

The first is indirection. The method-table modules route operations through function pointers, which is cheap compared with allocation or traversal cost but still visible in tight loops.

The second is storage shape. `ARRAY_LIST`, `BINARY_HEAP`, and `deque` keep data in contiguous buffers. `LINKED_LIST`, `TRIE`, `FIBONACCI_HEAP`, and tree nodes pay pointer-chasing cost instead.

The third is growth policy. Array-backed structures double capacity. Hash maps resize when the load factor exceeds `0.75`.

## How to Measure Locally

If you need numbers for a workload, add one small benchmark program next to the tests, build it with the release preset, and report:

1. compiler and version
2. optimization flags
3. CPU model
4. container size or graph density
5. operations per run
6. median or average over repeated runs

The shortest command path, when your CMake build supports presets, is:

```bash
cmake --preset release
cmake --build build/release
```

Then run the benchmark binary from `build/release/bin`.

## Current Gap

The missing benchmark harness is a documentation gap and a tooling gap. Until the repo ships reproducible benchmark code, performance claims in prose should stay at the level of implementation cost and access-pattern guidance.
