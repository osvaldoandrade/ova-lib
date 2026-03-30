# Architecture

ova-lib is organized as one CMake project, 16 public headers, 32 library source files, and 22 unit-test programs. The repo does not use a single API pattern. Some modules expose opaque handles, while others expose structs with public fields and method pointers. The documentation in this repo follows the code as shipped, not an abstract pattern that only some modules use.

## Repository Layout

The top level is split into 5 working areas. `include/` holds the public headers. `src/` holds the implementation files grouped by subsystem. `test/` contains one focused test program per area. `docs/` holds the canonical prose. `CMakeLists.txt` and `CMakePresets.json` define the build, test, install, and packaging flow.

The umbrella header `include/ova.h` re-exports the main module headers. `include/types.h` carries the shared comparator and hash typedefs that several modules reuse.

## Interface Shapes

The library uses 2 public shapes.

The first shape is an opaque handle with free functions. `graph`, `tree`, `set`, `trie`, and `bloom_filter` are declared as incomplete types in headers, and callers work through standalone functions such as `graph_add_edge`, `tree_insert`, `set_add`, `trie_search`, and `bloom_filter_add`.

The second shape is a public struct with method pointers. `list`, `queue`, `heap`, `stack`, `matrix`, `vector`, `solver`, `lp_problem`, and `map` expose fields in the public headers and route most operations through function pointers such as `insert`, `get`, `pop`, `destroy`, or `solve`.

That split matters for documentation and for user code. A page that claims every public type is opaque is wrong for the current headers.

## Build Topology

`CMakeLists.txt` defines 2 library targets from the same source set: `ova_lib_static` and `ova_lib_shared`. Both link against `libm`. The project enables CTest through `include(CTest)` and generates 22 test executables named `test_*`.

The build outputs follow one layout:

| Output | Directory |
| --- | --- |
| Static and shared libraries | `<build>/lib` |
| Test executables | `<build>/bin` |
| Copied public headers for the build tree | `<build>/include` |

`cmake --install` installs headers into `<prefix>/include`, libraries into `<prefix>/lib`, and the generated `ova-lib.pc` file into `<prefix>/lib/pkgconfig`.

## Ownership Model

The containers and graph-like modules own their internal buffers, nodes, buckets, matrices, or adjacency storage. They do not free caller payloads. A `list`, `queue`, `heap`, `map`, `set`, `tree`, or `trie` stores raw pointers. The caller decides whether those pointers refer to borrowed memory, stack values, or heap allocations.

Return values follow the same split. A container destructor frees only the container. Functions that return pointers from inside a structure usually return the stored payload pointer, not a copy.

## Concurrency

The repo is single-threaded except for one path in the hash map. `create_map(HASH_TABLE, ...)` allocates a `pthread_mutex_t` and wraps `put`, `get`, and `remove` with one coarse lock. `create_map(HASH_MAP, ...)` skips the mutex. No other public module adds internal synchronization.

## Diagrams

The diagram sources stay under [`docs/diagrams/`](diagrams/). They are useful as a supplement, but they are not the source of truth for API claims. The headers and tests are.
