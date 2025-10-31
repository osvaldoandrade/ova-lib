# Architecture

## Repository layout
- `include/` exposes public headers for containers, matrices, sorting, queues, heaps, stacks, and the LP solver orchestration. `ova.h` aggregates the exports so downstream projects can include a single header.
- `src/` mirrors the public API by module. Implementations hide their internal structs in translation units to keep ABI stable and allow swapping strategies without touching callers.
- `test/` hosts the unit executables wired through CTest.
- `CMakeLists.txt` builds both shared and static targets from the same object files and stages headers into the build tree for installation.

## Build pipeline
1. Configure with CMake ≥3.10. The script sets C11 mode, enables position independent code for shared builds, and surfaces options for toggling sanitizers during local debugging.
2. `cmake --build` produces `libova_lib.so` and `libova_lib.a` plus all module-focused unit tests under `${CMAKE_BINARY_DIR}/test`.
3. `ctest --output-on-failure` executes each module test binary; failures bubble up through CI because of the `ENABLE_TESTING` directive.
4. `cmake --install` copies headers into `${CMAKE_INSTALL_PREFIX}/include/ova_lib` and libraries into the matching `lib` directory. The generated install script honors DESTDIR for package managers.

## Memory ownership
- Every abstract handle returned to users (`list`, `heap`, `queue`, `map`, `matrix`, `sorter`, `lp_problem`, `solver`) owns only its internal buffers. Payload pointers remain caller-managed to avoid double frees and to make embedding feasible.
- Factories allocate opaque `impl` structs and assign function pointers immediately. Destructors validate internal pointers before freeing memory to remain resilient to double-destroy attempts (for example, `matrix_destroy` iterates rows before freeing the matrix wrapper).
- Dynamic buffers grow exponentially where throughput matters: arrays double capacity, heaps double their storage, and hash maps double the bucket array when the load factor exceeds 0.75.

## Error handling and invariants
- Constructors bail out early on allocation failures, cleaning partially built structures (e.g., matrix creation unwinds row allocations on failure).
- Many operations validate indices and bounds. Array-backed containers guard against negative indexes and size overflows before invoking `memmove`.
- Solver routines check for NULL handles before progressing. The simplex driver returns `INFEASIBLE` when inputs are incomplete, and the LP problem helpers refuse to mutate problems whose backing matrices failed to resize.

## Thread safety model
- All components are single-threaded by default. The hash map factory is the only API that conditionally allocates a `pthread_mutex_t`, turning on coarse-grained locking around `put`, `get`, and `remove` when `map_type` is `HASH_TABLE`.
- Iteration APIs are intentionally absent to avoid exposing unsafe traversal semantics; clients iterate by re-hashing keys or using container-specific visitors they implement externally.

## Extensibility surface
- All container types expose a uniform vtable (function pointer table) so alternative implementations can be injected without recompiling dependents. For example, `create_heap` can return either a binary heap or a Fibonacci heap while the consumer calls `put`, `pop`, `peek`, and `size` generically.
- Solver extension points include placeholder enums for Lagrangean relaxation, branch-and-bound, and branch-and-cut. Stubs compile but return NULL until their algorithms are provided, ensuring the public API is forward-compatible.
- Hash functions are pluggable through the `hash_func_t` typedef; users can select from the built-ins or pass a custom function when creating the map.
