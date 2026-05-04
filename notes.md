# Perf Improver memory — osvaldoandrade/ova-lib

## Validated commands (Linux)
- Build: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel`
- Tests: `ctest --test-dir build --output-on-failure` (~0.07s, 29 tests)
- ASan: `cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"`
  - Run with `LD_PRELOAD=$(gcc -print-file-name=libasan.so) ASAN_OPTIONS=detect_leaks=0 ./build-asan/bin/test_X` (LSan not in agent sandbox; CI runs full ASan).
- Lint: cppcheck (NOT installed in agent env; CI runs it)
- Bench: ad-hoc `gcc -O2 -Iinclude bench.c build/lib/libova_lib.a -lpthread -lm`; binaries land in `/tmp/gh-aw/agent/`

## Repo notes
- C11 library, public API in `include/`, impls in `src/<module>/`.
- No benchmark harness exists; ad-hoc bench programs go in `/tmp/gh-aw/agent/`.
- CI: .github/workflows/main.yml — multi-arch build/test, ASan, Valgrind, coverage, cppcheck.
- treats unused static functions as errors (-Wunused-function): clean up dead helpers.
- linked_list has cursor cache; bloom_filter dual FNV-1a; skip_list xorshift+ctz+co-alloc.
- linked_queue freelist (cap 4096); deque resize uses two memcpys.
- Dijkstra and Prim both use bump-pointer arenas (pq_node / prim_cand) — same chunk shape.
- IMPORTANT: glibc-style LCG (mul 1103515245 + 12345) low bit is period-2.

## Optimization backlog
- DONE 2026-04-29: sort.c snapshot-buffer (PR #127 merged).
- DONE 2026-04-29: linked_list cursor cache (PR #136 merged).
- DONE 2026-04-29: bloom_filter fused dual FNV-1a (PR #138 merged).
- DONE 2026-04-30: skip_list xorshift+ctz+co-alloc (PR #140 merged).
- DONE 2026-05-01: linked_queue node freelist (PR #143 merged).
- DONE 2026-05-02: deque resize memcpy x2 (PR #147 OPEN — needs maintainer review).
- DONE 2026-05-03: dijkstra pq_node bump-arena (PR #149 merged).
- DONE 2026-05-04: Prim heap bump-arena candidates (this run — PR opened on perf-assist/prim-cand-arena).
- TODO: hash_map resize_and_rehash recomputes hash per entry (API constraint; needs discussion).
- TODO: graph_algorithms.c — Tarjan recursive; bellman_ford untouched.
- TODO: matrix.c (1005 LOC) — SIMD opportunities.
- TODO: benchmark harness (CTest perf labels or Google Benchmark).
- TODO: array_list / sorted_list capacity growth + front-insert.
- TODO: hash_map / hash_set load factor / bernstein hash on int keys.

## Activity issue
- 2026-04 #137 closed by maintainer 2026-05-01.
- 2026-05 #144 was closed by maintainer; new monthly activity issue created this run.

## Recent runs
- 2026-04-30 20:52 UTC: skip_list RNG + co-alloc PR #140.
- 2026-05-01 20:50 UTC: linked_queue freelist PR #143 (merged).
- 2026-05-02 20:49 UTC: deque resize two-memcpy PR #147 (still open).
- 2026-05-03 20:49 UTC: dijkstra pq_node bump-arena PR #149 (merged).
- 2026-05-04 20:55 UTC: Prim bump-arena candidates: V=2k deg=16 6.35→2.52 (2.5×); V=2k deg=32 14.8→3.8 (3.9×); V=5k deg=16 21→7.4 (2.9×); V=500 deg=100 8.4→1.5 (5.6×); V=800 deg=50 ADJ_MATRIX 8.6→3.6 (2.4×).
