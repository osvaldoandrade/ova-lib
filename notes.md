# Perf Improver memory — osvaldoandrade/ova-lib

## Validated commands (Linux)
- Build: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel`
- Tests: `ctest --test-dir build --output-on-failure` (~0.1s, 29 tests)
- ASan: `cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"`
- Lint: `cppcheck --language=c --std=c11 --enable=warning,performance,portability -I include src include`
- Bench: ad-hoc gcc programs against `build/lib/libova_lib.a`

## Repo notes
- C11 library, public API in `include/`, impls in `src/<module>/`.
- No benchmark harness exists; ad-hoc bench programs go in `/tmp/gh-aw/agent/`.
- CI: .github/workflows/main.yml — multi-arch build/test, ASan, Valgrind, coverage, cppcheck.
- ASan runs locally fail with "runtime not first" because shared lib path; tests pass when launched with LD_PRELOAD of libasan.
- `list` interface in include/list.h has `_type` field used for clone dispatch; usable for type-aware fast paths.

## Optimization backlog
- DONE 2026-04-29: sort.c snapshot-buffer rewrite (PR #pending).
- TODO: linked_list iteration via index is O(n²) for min/max/copy in sort.c. Could expose iterator.
- TODO: hash_map resize_and_rehash recomputes hash for each entry; consider caching hash in map_entry.
- TODO: bloom_filter / skip_list — review hash quality and RNG cost.
- TODO: graph_algorithms.c (1179 LOC) — review hot loops.
- TODO: matrix.c (981 LOC) — SIMD opportunities (test_vector_simd hints work started).
- TODO: Add proper benchmark harness (CTest perf labels or Google Benchmark) so CI can detect regressions.

## Activity issue
- Monthly issue not yet created.
