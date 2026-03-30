# Documentation Review

This review covers the library repo and the wiki repo as they stood on 2026-03-29. The focus is documentation accuracy, link integrity, and alignment between prose, headers, tests, and build files.

## Scope and Method

The review used 4 sources of truth: `include/*.h`, `src/`, `test/`, and the top-level build files. The markdown tree was treated as correct only when those sources agreed with it.

The pass covered 16 markdown pages in the library repo and 6 pages in the wiki repo. Link integrity was checked with a local relative-link audit. API claims were checked against headers and spot-checked against tests.

## Findings Before the Rewrite

The first finding was link failure. `Home.md` in the wiki contained 15 broken links. Every broken link was on that page.

The second finding was narrative drift. The 5 non-home wiki pages described an architecture built around opaque handles and sealed implementations. The shipped headers do not follow that rule uniformly. `list`, `queue`, `heap`, `stack`, `matrix`, `map`, `solver`, and `lp_problem` expose public fields or method tables.

The third finding was routing failure. The repo and the wiki both tried to act as the full manual. That duplication raised maintenance cost and made drift inevitable.

The fourth finding was evidence failure. `docs/benchmarks.md` contained detailed timing claims, but the repo did not contain benchmark code that could reproduce them.

The fifth finding was stale review material. The previous `recommendations.md` and `recommendations-summary.md` described a roadmap that no longer matched the shipped library surface, because several named features are already present while others remain partial.

## Changes Applied in This Pass

The repo now carries the canonical manual. `README.md` was reduced to an entry document with build, test, install, module, and navigation details. The subsystem pages in `docs/` were rewritten around the current public API and current ownership rules.

The wiki was converted into an orientation layer. Its home page now links only to valid wiki pages or to stable GitHub URLs in the main repo. The long-form spec language was removed from wiki pages and replaced with short, directional pages.

The benchmark page was rewritten as a performance-notes page that states what the code structure implies and what the repo still lacks. The review pages were repurposed into dated audit records so they stop competing with evergreen documentation.

## Gaps That Remain After the Rewrite

The first remaining gap is in the solver layer. `SolverType` declares `SOLVER_BRANCH_AND_CUT`, `SOLVER_BRANCH_AND_BOUND`, `SOLVER_LAGRANGEAN_SIMPLEX`, and `SOLVER_SIMPLEX`, but only simplex is implemented. The docs now say that plainly.

The second remaining gap is in the sorter lifetime model. `create_sorter` allocates a helper object, but the public header does not provide a destroy function. The docs now note the gap instead of hiding it.

The third remaining gap is performance evidence. The repo still needs a benchmark harness if it wants to publish timing tables as part of the manual.

## Result

After the rewrite, the docs follow 3 rules.

1. The repo docs describe only shipped APIs and shipped behavior.
2. The wiki acts as a map, not as a second manual.
3. Pages make claims only when the repo can support them with code, tests, or build files.
