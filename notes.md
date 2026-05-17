# Perf Improver memory — osvaldoandrade/ova-lib

## Validated commands (Linux)
- Build: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel`
- Tests: `ctest --test-dir build --output-on-failure` (~0.08s, 29 tests)
- ASan: `cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"`
  - Run with `LD_PRELOAD=$(gcc -print-file-name=libasan.so) ASAN_OPTIONS=detect_leaks=0 ./build-asan/bin/test_X`
- Lint: cppcheck (NOT installed in agent env; CI runs it)
- Bench: ad-hoc `gcc -O2 -Iinclude bench.c build/lib/libova_lib.a -lpthread -lm`; binaries land in `/tmp/gh-aw/agent/`

## Repo notes
- C11 library, public API in `include/`, impls in `src/<module>/`.
- No benchmark harness exists; ad-hoc bench programs go in `/tmp/gh-aw/agent/`.
- CI: .github/workflows/main.yml — multi-arch build/test, ASan, Valgrind, coverage, cppcheck.
- treats unused static functions as errors (-Wunused-function): clean up dead helpers.
- Patterns landed: linked_list cursor cache; bloom_filter dual FNV-1a fused; skip_list xorshift+ctz+co-alloc; linked_queue freelist (cap 4096); deque resize 2 memcpys; Dijkstra/Prim bump-arena; hash_map per-map map_entry freelist (cap 65536); bellman_ford SPFA active-set bitmap; Tarjan SCC iterative explicit-stack; BFS/DFSi/CC raw int[] queue+stack with hoisted present[]/row.
- IMPORTANT: glibc-style LCG (mul 1103515245 + 12345) low bit is period-2.
- map_impl in src/map/map_internal.h.
- graph adj_matrix is double*, sentinel GRAPH_NO_EDGE = INFINITY.

## Optimization backlog
- DONE 2026-04..05: sort snapshot, linked_list cursor, bloom dual-FNV, skip_list co-alloc, linked_queue freelist, deque memcpy, Dijkstra arena, Prim arena, hash_map freelist, bellman_ford SPFA — all merged.
- DONE 2026-05-14: Tarjan SCC iterative explicit-stack DFS (PR opened on perf-assist/tarjan-iterative).
- DONE 2026-05-15: BFS/DFSi/CC raw int[] queue+stack (PR opened on perf-assist/graph-bfs-dfs-array).
- DONE 2026-05-16: topological_sort int[] ring (PR opened on perf-assist/topo-sort-array).
- DONE 2026-05-17: has_cycle iterative explicit-stack (PR opened on perf-assist/has-cycle-iterative).
- TODO: hash_map resize_and_rehash recomputes hash per entry (API constraint).
- TODO: matrix.c (1005 LOC) — SIMD opportunities.
- TODO: benchmark harness (CTest perf labels or Google Benchmark).
- TODO: array_list / sorted_list capacity growth + front-insert.
- TODO: hash_set bench (likely already benefits via map).
- TODO: profile graph BFS/DFS iterative for arena/hoisting wins.

## Activity issue
- 2026-04 #137 closed.
- 2026-05 #144 closed.
- 2026-05 #153 closed by maintainer 2026-05-08.
- 2026-05 (this run) reopened — see latest run.

## Recent runs
- 2026-05-05 20:52 UTC: hash_map map_entry freelist PR #156 (merged 2026-05-08).
- 2026-05-06 20:58 UTC: bellman_ford SPFA active-set PR (merged as #158 2026-05-08).
- 2026-05-08: maintainer merged #147, #156, #158 and closed #153.
- 2026-05-14 19:14 UTC: Tarjan SCC iterative explicit-stack DFS PR opened (perf-assist/tarjan-iterative). Chain list N=10k 0.77->0.59 (1.30x), N=50k 4.91->2.96 (1.66x); chain matrix N=2k 6.39->5.56 (1.15x), N=4k 24.98->22.22 (1.12x). Random sparse list N=50k 22.87->17.55 (1.30x), matrix N=2k 8.21->5.80 (1.41x). Stress: 1M-vertex chain ~63 ms (recursive baseline overflows 8 MB stack). 29/29 ctest pass; ASan-clean. Reopened monthly activity issue.
- 2026-05-15 19:08 UTC: BFS/DFSi/CC raw int[] queue+stack PR opened (perf-assist/graph-bfs-dfs-array). DFSi chain list 50k 1.301->0.654 (1.99x); BFS random list 50k d=4 9.02->5.30 (1.70x); DFSi random list 50k d=4 12.33->7.81 (1.58x); CC undirect list 50k d=4 18.74->12.36 (1.52x); BFS chain matrix 2k 4.99->3.44 (1.45x). 29/29 ctest pass; ASan-clean. Updated activity issue #173.
- 2026-05-16 18:57 UTC: topological_sort int[] ring PR opened (perf-assist/topo-sort-array). chain list 50k 1.250->0.885 (1.41x); chain list 200k 6.395->5.320 (1.20x); chain matrix 2k 9.186->6.646 (1.38x); dag list 200k d=4 29.02->24.42 (1.19x); dag matrix 2k d=4 9.38->7.14 (1.31x). 29/29 ctest pass; ASan-clean.
- 2026-05-17 18:57 UTC: has_cycle iterative explicit-stack PR opened (perf-assist/has-cycle-iterative). Throughput within noise on chain/DAG (50k-200k); primary win is robustness — 1M-vertex chain completes in ~24 ms while recursive baseline overflows 8 MB stack. Pattern: cycle_frame stack sized to vertex_capacity, hoist cap/present/adj_lists/matrix row, unsigned-cast bounds check. 29/29 ctest pass; ASan-clean on test_graph_algorithms.
