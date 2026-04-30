# Perf Improver memory — osvaldoandrade/ova-lib

## Validated commands (Linux)
- Build: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel`
- Tests: `ctest --test-dir build --output-on-failure` (~0.1s, 29 tests)
- ASan: `cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"`
- Lint: cppcheck (NOT installed in agent env; CI runs it)
- Bench: ad-hoc gcc programs against `build/lib/libova_lib.a`; binaries land in `/tmp/gh-aw/agent/`

## Repo notes
- C11 library, public API in `include/`, impls in `src/<module>/`.
- No benchmark harness exists; ad-hoc bench programs go in `/tmp/gh-aw/agent/`.
- CI: .github/workflows/main.yml — multi-arch build/test, ASan, Valgrind, coverage, cppcheck.
- linked_list has cursor cache (cursor_node, cursor_index); get() picks closest of head/tail/cursor.
- bloom_filter uses FNV-1a dual hashing; both seeds updated in single pass via fnv1a64_dual.
- skip_list now uses xorshift32 + __builtin_ctz for level selection; node + forward[] co-allocated.
- IMPORTANT: glibc-style LCG (mul 1103515245 + 12345) low bit is period-2 — never use bit 0 alone for coin flips. Caused skip_list to degenerate to linked list.

## Optimization backlog
- DONE 2026-04-29: sort.c snapshot-buffer rewrite (PR #127 merged).
- DONE 2026-04-29: linked_list cursor cache (PR #136 pending).
- DONE 2026-04-29: bloom_filter fused dual FNV-1a (PR #138 pending).
- DONE 2026-04-30: skip_list xorshift + ctz + co-alloc forward (PR pending; this run).
- TODO: hash_map resize_and_rehash recomputes hash each entry. API constraint: hash_func returns modded index. Would need API change. Defer / discuss.
- TODO: graph_algorithms.c (1179 LOC) — review hot loops (BFS/DFS/Dijkstra likely).
- TODO: matrix.c (981 LOC) — SIMD opportunities (test_vector_simd hints work started).
- TODO: Add proper benchmark harness (CTest perf labels or Google Benchmark) so CI can detect regressions.
- TODO: array_list / sorted_list — capacity growth strategy and front-insert.
- TODO: hash_map / hash_set — measure load factor, collision rate; bernstein hash quality on integer keys.

## Activity issue
- 2026-04 monthly activity issue #137 — updated this run.

## Recent runs
- 2026-04-29 14:10 UTC: linked_list cursor PR #136.
- 2026-04-29 20:52 UTC: bloom_filter dual-hash PR #138 (16B 1.13–1.27×, 128B 1.59–1.65×).
- 2026-04-30 20:52 UTC: skip_list RNG + co-alloc PR (5k: 68–170×; 100k: >4000× vs degenerate baseline).
