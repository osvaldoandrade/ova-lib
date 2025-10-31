# ova-lib Documentation Home

## Purpose
This wiki captures the implementation details, trade-offs, and operational characteristics of the ova-lib container and algorithm suite. Use these pages to understand:

- How each subsystem is wired in the codebase (`src/` and `include/`).
- Time and space behavior of the public APIs.
- Memory ownership contracts and extension points when embedding the library in larger systems.

## Navigation
- [Architecture](architecture.md): build pipeline, directory layout, ownership rules, and extension hooks.
- [Containers](containers.md): lists, stacks, queues, heaps, maps, and hashing internals.
- [Matrix utilities](matrix.md): lifecycle helpers, arithmetic routines, and numerical stability notes.
- [Simplex solver](solver-simplex.md): tableau construction, pivot strategy, and termination semantics.
- [Sorting utilities](sorting.md): quicksort driver, helper routines, and algorithmic guarantees.

## Conventions
- All documentation references the C identifiers exactly as they appear in the headers under `include/` and the implementations in `src/`.
- Complexity notes follow big-O notation and assume the comparator cost is O(1) unless stated otherwise.
- Payload pointers (`void *`) remain caller-owned; destructors only release internal buffers.

## Getting Started
1. Read the [architecture overview](architecture.md) to understand factory patterns and error handling.
2. Dive into the [containers guide](containers.md) for data structure specifics and hashing strategy selection.
3. Use the remaining pages for subsystem deep-dives when integrating or extending the library.
