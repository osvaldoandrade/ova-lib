# Architecture

## Diagrams

Visual representations of the library's design live in [`docs/diagrams/`](diagrams/).
They are split into five files, each targeting a different aspect of the architecture:

| File | Tool | Coverage |
|------|------|----------|
| [`diagrams/factory-patterns.puml`](diagrams/factory-patterns.puml) | PlantUML | Factory functions and the vtable-backed types they produce |
| [`diagrams/module-dependencies.dot`](diagrams/module-dependencies.dot) | Graphviz | `#include` and logical dependencies between every module |
| [`diagrams/memory-layouts.puml`](diagrams/memory-layouts.puml) | PlantUML | In-memory field layout of the key public and internal structs |
| [`diagrams/algorithm-flowcharts.md`](diagrams/algorithm-flowcharts.md) | Mermaid | Step-by-step control flow for heap, hash-map, sort, graph, simplex, and bloom-filter algorithms |
| [`diagrams/composition-relationships.puml`](diagrams/composition-relationships.puml) | PlantUML | Vtable interfaces, concrete implementations, and cross-module composition |

### Rendering locally

**PlantUML** (requires Java):
```sh
plantuml docs/diagrams/factory-patterns.puml
plantuml docs/diagrams/memory-layouts.puml
plantuml docs/diagrams/composition-relationships.puml
```

**Graphviz**:
```sh
dot -Tsvg docs/diagrams/module-dependencies.dot -o docs/diagrams/module-dependencies.svg
```

**Mermaid** diagrams in `algorithm-flowcharts.md` are rendered automatically by
GitHub and any Markdown viewer with Mermaid support.

---

## Repository layout
The repository divides public interfaces and internal logic by mirroring module boundaries. Public headers live in `include/`, where containers, matrix helpers, sorting utilities, queues, heaps, stacks, and the linear-programming orchestration are exposed. A single umbrella header, `ova.h`, re-exports those declarations so downstream projects can adopt the library with a single include. Implementation files in `src/` shadow the public API while hiding concrete structures within translation units, which keeps the ABI stable and makes it possible to swap strategies without touching callers. Tests reside in `test/`, each compiled into a dedicated executable that CTest discovers automatically. The top-level `CMakeLists.txt` compiles both shared and static variants from a shared object library and stages headers into the build tree for installation.

## Build pipeline
Building starts by configuring the project with CMake 3.10 or newer. The configuration script enforces C11 mode, flips on position-independent code when shared libraries are requested, and exposes options that enable sanitizers for local debugging. A subsequent `cmake --build` invocation produces `libova_lib.so`, `libova_lib.a`, and module-focused unit tests under `${CMAKE_BINARY_DIR}/test`. Running `ctest --output-on-failure` executes each unit binary and propagates failures back to continuous integration because testing is wired in through `ENABLE_TESTING`. Installation relies on `cmake --install`, which copies headers into `${CMAKE_INSTALL_PREFIX}/include/ova_lib` and libraries into the matching `lib` directory while honoring `DESTDIR` so package managers can stage files into a sandbox.

## Memory ownership
Every handle returned to callers—lists, heaps, queues, maps, matrices, sorters, linear-programming problems, and solvers—owns only its internal buffers. Payload pointers stay under the caller’s control to avoid double frees and to support embedded deployments. Factory functions allocate opaque implementation structures, wire their function tables immediately, and guard against double-destroy attempts by validating internal pointers before releasing memory. Dynamic buffers grow geometrically when throughput matters: arrays and heaps double capacity during expansion, and hash maps double their bucket array when the load factor rises above 0.75.

## Error handling and invariants
Constructors abort early when allocations fail and clean up partially built structures before returning. Array-backed containers validate indices and bounds before performing `memmove` so invalid accesses cannot corrupt memory. Solver routines reject NULL handles before progressing. The simplex driver reports `INFEASIBLE` whenever required inputs are missing, and the linear-programming helpers refuse to mutate problems whose backing matrices could not resize correctly.

## Thread safety model
Components are single-threaded unless stated otherwise. The hash map factory optionally allocates a `pthread_mutex_t` when the requested `map_type` is `HASH_TABLE`; the resulting lock wraps `put`, `get`, and `remove` to provide coarse-grained protection. Iteration APIs are intentionally absent because exposing internal traversal semantics would invite data races in concurrent scenarios. Clients that need iteration perform their own re-hashing or implement visitors tailored to their use case.

## Extensibility surface
Container types expose uniform vtables so alternative implementations can be injected without recompiling dependents. For example, `create_heap` can yield a binary heap today while future versions can introduce a Fibonacci heap under the same interface, leaving callers to invoke `put`, `pop`, `peek`, and `size` without modification. Solver extension points include placeholder enums for Lagrangean relaxation, branch-and-bound, and branch-and-cut. The stubs compile yet return NULL until full algorithms arrive, preserving forward compatibility. Hash functions remain pluggable through the `hash_func_t` typedef, allowing applications to pick one of the built-in strategies or supply their own routine during map creation.
