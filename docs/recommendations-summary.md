# Documentation Review Summary

This file records the documentation pass completed on 2026-03-29 across 2 repos: `ova-lib` and `ova-lib.wiki`.

## Baseline

The pass started from 4 measurable problems.

| Finding | Count |
| --- | ---: |
| Broken wiki links | 15 |
| Wiki pages in scope | 6 |
| Repo markdown pages in scope | 16 |
| Test executables used as source-of-truth anchors | 22 |

All 15 broken links were on the wiki home page. The failure split was simple: 3 links pointed at invalid wiki-local routes, and 12 links tried to reach `docs/*.md` through the wiki repo with paths that could never resolve on GitHub.

## Main Decisions

The repo now owns the long-form narrative. The wiki now acts as a short entry layer. Pages that claimed interfaces not present in `include/*.h` were rewritten from the shipped API rather than from older design notes.

The benchmark page dropped unsupported timing tables because the repo does not ship a benchmark harness. The review pages were rewritten as dated audit records instead of product-roadmap documents.

## Remaining Gaps Worth Tracking

Three gaps remain visible after the doc rewrite.

1. `solver.h` declares 4 solver types, but only `SOLVER_SIMPLEX` is wired in the implementation.
2. `sort.h` exposes `create_sorter` without a public destructor.
3. The repo still lacks benchmark source code to back performance numbers with reproducible runs.

The detailed audit lives in [recommendations.md](recommendations.md).
