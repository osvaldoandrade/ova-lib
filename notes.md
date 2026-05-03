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
- linked_list has cursor cache (cursor_node, cursor_index); get() picks closest of head/tail/cursor.
- bloom_filter uses FNV-1a dual hashing; both seeds updated in single pass via fnv1a64_dual.
- skip_list uses xorshift32 + __builtin_ctz for level selection; node + forward[] co-allocated.
- linked_queue has a bounded per-queue freelist (cap 4096) — enqueue/dequeue reuse nodes.
- queue_impl gained `freelist`, `freelist_size`; heap_queue uses calloc so its zero-init is fine.
- deque resize uses two memcpys on the wrapped live range (one when contiguous).
- IMPORTANT: glibc-style LCG (mul 1103515245 + 12345) low bit is period-2 — never use bit 0 alone for coin flips.

## Optimization backlog
- DONE 2026-04-29: sort.c snapshot-buffer rewrite (PR #127 merged).
- DONE 2026-04-29: linked_list cursor cache (PR #136 merged).
- DONE 2026-04-29: bloom_filter fused dual FNV-1a (PR #138 merged).
- DONE 2026-04-30: skip_list xorshift + ctz + co-alloc forward (PR #140 merged).
- DONE 2026-05-01: linked_queue node freelist (PR #143 merged).
- DONE 2026-05-02: deque resize memcpy x2 (PR #147 open).
- DONE 2026-05-03: dijkstra pq_node bump-arena (PR pending; this run; ~1.09–1.16×).
- TODO: prim_push_edges allocates graph_weighted_edge per push — same arena idea.
- TODO: hash_map resize_and_rehash recomputes hash each entry. API constraint: hash_func returns modded index. Defer / discuss.
- TODO: graph_algorithms.c — Tarjan still recursive; bellman_ford untouched.
- TODO: matrix.c (1005 LOC) — SIMD opportunities (test_vector_simd hints prior work).
- TODO: Add proper benchmark harness (CTest perf labels or Google Benchmark) so CI can detect regressions.
- TODO: array_list / sorted_list — capacity growth strategy and front-insert.
- TODO: hash_map / hash_set — measure load factor, collision rate; bernstein hash quality on integer keys.

## Activity issue
- 2026-04 #137 closed by maintainer 2026-05-01.
- 2026-05 monthly activity issue #144 (open).

## Recent runs
- 2026-04-29 14:10 UTC: linked_list cursor PR #136.
- 2026-04-29 20:52 UTC: bloom_filter dual-hash PR #138 (16B 1.13–1.27×, 128B 1.59–1.65×).
- 2026-04-30 20:52 UTC: skip_list RNG + co-alloc PR #140 (5k: 68–170×; 100k: >4000× vs degenerate baseline).
- 2026-05-01 20:50 UTC: linked_queue freelist PR #143 (mixed 15.0→5.3 ms; merged).
- 2026-05-02 20:49 UTC: deque resize two-memcpy PR #147 (cap=1024 12.05→2.58 ms ~4.7×, cap=1M 54.6→18.7 ms ~2.9×).
- 2026-05-03 20:49 UTC: dijkstra pq_node bump-arena (V=2k deg=32 1.45→1.27 ms; V=5k deg=16 3.14→2.74 ms; ~1.09–1.16×).
