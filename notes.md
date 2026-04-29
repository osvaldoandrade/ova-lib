# Perf Improver memory — osvaldoandrade/ova-lib

## Validated commands (Linux)
- Build: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel`
- Tests: `ctest --test-dir build --output-on-failure` (~0.1s, 29 tests)
- ASan: `cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"`
- Lint: cppcheck (NOT installed in agent env; CI runs it)
- Bench: ad-hoc gcc programs against `build/lib/libova_lib.a`; binaries land in `build/bin/test_*`

## Repo notes
- C11 library, public API in `include/`, impls in `src/<module>/`.
- No benchmark harness exists; ad-hoc bench programs go in `/tmp/gh-aw/agent/`.
- CI: .github/workflows/main.yml — multi-arch build/test, ASan, Valgrind, coverage, cppcheck.
- ASan runs locally fail with "runtime not first" because shared lib path; tests pass when launched with LD_PRELOAD of libasan.
- `list` interface in include/list.h has `_type` field used for clone dispatch; usable for type-aware fast paths.
- linked_list now has cursor cache in impl (cursor_node, cursor_index); get() picks closest of head/tail/cursor.
- list_snapshot() in sort.c uses get(i) repeatedly — was the bottleneck before linked_list cursor cache; now O(n) too.

## Optimization backlog
- DONE 2026-04-29: sort.c snapshot-buffer rewrite (PR #127 merged).
- DONE 2026-04-29: linked_list cursor cache (PR pending). Made min/max/min_max/copy on linked_list ~2700x faster at N=40k.
- TODO: hash_map resize_and_rehash recomputes hash each entry. API constraint: hash_func returns modded index. Would need API change or storing raw 64-bit hash separately. Defer.
- TODO: bloom_filter / skip_list — review hash quality and RNG cost.
- TODO: graph_algorithms.c (1179 LOC) — review hot loops (BFS/DFS/Dijkstra likely).
- TODO: matrix.c (981 LOC) — SIMD opportunities (test_vector_simd hints work started).
- TODO: Add proper benchmark harness (CTest perf labels or Google Benchmark) so CI can detect regressions.
- TODO: array_list — check insert at front/middle, dynamic capacity growth strategy.
- TODO: hash_map / hash_set — measure load factor, collision rate; bernstein hash quality on integer keys.

## Activity issue
- 2026-04 monthly activity issue not yet created — create on this run.

## Recent runs
- 2026-04-29 ~14:00 UTC: linked_list cursor PR. Bench numbers stored above.
