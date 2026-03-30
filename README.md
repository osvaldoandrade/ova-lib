# ova-lib

ova-lib is a C11 library of data structures and numeric helpers. The tree on `main` exposes 14 module headers plus `ova.h` and `types.h`, builds 32 library translation units, and runs 22 unit-test executables through CTest. The library keeps payload ownership with the caller and stores user data as `void *` across the container APIs.

## What Ships

The current public surface groups into 8 areas.

| Area | Headers | Notes |
| --- | --- | --- |
| Linear containers | `list.h`, `queue.h`, `stack.h`, `deque.h` | Array, linked, sorted, FIFO, priority, and double-ended flows |
| Priority structures | `heap.h`, `sort.h` | Binary heap, Fibonacci heap, and list-based sorting helpers |
| Keyed storage | `map.h`, `set.h`, `tree.h`, `trie.h` | Hash table, ordered trees, sets, and prefix lookup |
| Numeric helpers | `matrix.h`, `solver.h` | Matrix arithmetic, vectors, and a simplex solver |
| Graphs | `graph.h` | Directed or undirected graphs with two representations |
| Probabilistic lookup | `bloom_filter.h` | Tunable false-positive membership checks |
| Umbrella include | `ova.h` | Re-exports the main headers |
| Shared types | `types.h` | Comparator and hash typedefs |

## Documentation

The repo docs are the canonical narrative. The wiki is a short map into this material.

- [Architecture overview](docs/architecture.md)
- [Usage guide](docs/best-practices.md)
- [Containers, heaps, maps, and deque](docs/containers.md)
- [Graphs](docs/graph.md)
- [Matrix and vector operations](docs/matrix.md)
- [Simplex solver](docs/solver-simplex.md)
- [Sorting helpers](docs/sorting.md)
- [Balanced trees](docs/trees.md)
- [Trie](docs/trie.md)
- [Set](docs/set.md)
- [Bloom filter](docs/bloom_filter.md)
- [Performance notes](docs/benchmarks.md)
- [Documentation review, 2026-03-29](docs/recommendations.md)
- [Review summary](docs/recommendations-summary.md)

## Build

`CMakeLists.txt` requires CMake 3.10 or newer and sets `C_STANDARD 11`. If your CMake build supports presets, the repo ships two configure presets in `CMakePresets.json`.

```bash
cmake --preset dev
cmake --build build/dev
```

For a release build:

```bash
cmake --preset release
cmake --build build/release
```

The manual path stays short:

```bash
mkdir -p build
cmake -S . -B build
cmake --build build
```

The build produces `libova_lib.a` and `libova_lib.so` under the build `lib/` directory. `cmake --install` copies headers into `<prefix>/include`, libraries into `<prefix>/lib`, and `ova-lib.pc` into `<prefix>/lib/pkgconfig`.

## Tests

CTest drives 22 executables listed in `CMakeLists.txt`.

```bash
ctest --test-dir build/dev --output-on-failure
```

If Valgrind is installed when CMake configures the tree, the `memcheck` target is available:

```bash
cmake --build build/dev --target memcheck
```

## Examples

This list example uses the array-backed implementation and reads values back by index.

```c
#include "list.h"

int main(void) {
    list *values = create_list(ARRAY_LIST, 4, NULL);
    int a = 3, b = 5;

    values->insert(values, &a, 0);
    values->insert(values, &b, 1);

    int *first = (int *)values->get(values, 0);
    int *second = (int *)values->get(values, 1);

    values->free(values);
    return first && second && *first == 3 && *second == 5 ? 0 : 1;
}
```

This queue example uses the heap-backed priority variant. The comparator decides what "higher priority" means.

```c
#include "queue.h"

static int int_cmp(const void *a, const void *b) {
    int lhs = *(const int *)a;
    int rhs = *(const int *)b;
    return (lhs > rhs) - (lhs < rhs);
}

int main(void) {
    queue *pq = create_queue(QUEUE_TYPE_PRIORITY, 8, int_cmp);
    int low = 2, high = 9;

    pq->enqueue(pq, &low);
    pq->enqueue(pq, &high);

    int *top = (int *)pq->dequeue(pq);
    pq->free(pq);
    return top && *top == 9 ? 0 : 1;
}
```

## License

ova-lib is licensed under the [Apache 2.0](LICENSE) license.
