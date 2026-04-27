# Integer Programming Solvers

This document describes the integer programming solvers available in ova-lib.
All three solvers build on the existing simplex LP solver to find optimal
integer-valued solutions.

## Solver Types

| Solver | Enum | Description |
|--------|------|-------------|
| Branch-and-Bound | `SOLVER_BRANCH_AND_BOUND` | Classic B&B with LP relaxation at each node |
| Branch-and-Cut | `SOLVER_BRANCH_AND_CUT` | B&B enhanced with Gomory cutting planes |
| Lagrangean Relaxation | `SOLVER_LAGRANGEAN_SIMPLEX` | Subgradient optimization with LP-based rounding |

## Usage

All integer programming solvers share the same API as the simplex solver:

```c
#include "solver.h"

/* Define an integer programming problem */
lp_problem *problem = create_problem(2, 3);

double objective[] = {3, 5};
problem->set_objective(problem, objective, PROBLEM_MAX);

double c1[] = {1, 2};
double c2[] = {-3, 1};
double c3[] = {1, -1};
problem->add_constraint(problem, c1, 14);
problem->add_constraint(problem, c2, 0);
problem->add_constraint(problem, c3, 2);

/* Choose a solver */
solver *s = create_solver(SOLVER_BRANCH_AND_BOUND);

matrix *tableau = NULL;
int result = s->solve(s, problem, &tableau);

if (result == OPTIMAL) {
    double x1 = problem->solution_value(problem, 0);
    double x2 = problem->solution_value(problem, 1);
    double obj = problem->objective_value(problem);
    /* x1 and x2 are guaranteed to be integer-valued */
}

/* Clean up */
if (tableau) tableau->free(tableau);
s->free(s);
problem->free(problem);
```

## Algorithm Details

### Branch-and-Bound (`SOLVER_BRANCH_AND_BOUND`)

The branch-and-bound algorithm solves integer programs by systematically
exploring a tree of LP relaxations:

1. **Root node**: Solve the LP relaxation. If the solution is already integer,
   return it.
2. **Branching**: Select the most fractional variable `x_j` and create two
   child subproblems:
   - Floor child: add constraint `x_j <= floor(val)`
   - Ceil child: substitute `x_j' = x_j - ceil(val)` (variable substitution to
     keep all RHS values non-negative for the simplex solver)
3. **Bounding**: Prune any node whose LP relaxation bound cannot improve the
   best known integer solution (incumbent).
4. **Fathoming**: When an integer-feasible solution is found that improves the
   incumbent, update the best solution.

The search is limited to `BB_MAX_NODES = 10000` nodes.

### Branch-and-Cut (`SOLVER_BRANCH_AND_CUT`)

Branch-and-cut extends branch-and-bound by adding **Gomory fractional cuts**
before branching at each node:

1. Solve the LP relaxation at the current node.
2. If the solution is fractional, generate Gomory cuts from the simplex tableau:
   - For each fractional row in the tableau, create a cut using the fractional
     parts of the tableau coefficients.
   - Add the cut as a new constraint to tighten the LP relaxation.
3. Re-solve the LP with the added cuts.
4. Repeat cut generation up to `BC_MAX_CUTS = 10` rounds.
5. If the solution is still fractional, branch as in branch-and-bound.

The cutting planes help tighten the LP relaxation, often reducing the number of
nodes that need to be explored.

### Lagrangean Relaxation (`SOLVER_LAGRANGEAN_SIMPLEX`)

The Lagrangean relaxation solver uses subgradient optimization to find feasible
integer solutions:

1. **LP relaxation**: Solve the original LP to get the continuous optimum.
2. **Subgradient loop** (up to `LR_MAX_ITER = 200` iterations):
   - Build a modified objective by adding Lagrangean penalty terms for
     constraint violations.
   - Solve the LP with the modified objective (keeping all original constraints).
   - Round the LP solution to the nearest integers.
   - Check feasibility of the rounded solution against the original constraints.
   - Update the Lagrange multipliers using subgradient steps.
3. **Step size control**: The step size is halved after 10 consecutive iterations
   without bound improvement.
4. Return the best feasible integer solution found.

This solver is a heuristic and may not find the global optimum, but it is
efficient for problems where rounding the LP solution yields good integer
candidates.

## Return Values

| Code | Constant | Meaning |
|------|----------|---------|
| 0 | `OPTIMAL` | An optimal integer solution was found |
| -1 | `UNBOUNDED` | The LP relaxation is unbounded |
| -2 | `INFEASIBLE` | No feasible integer solution exists (or could be found) |

## Helper Functions

- `is_integer(double value)` — Returns 1 if `value` is within `1e-6` of the
  nearest integer, 0 otherwise.

## Files

| File | Description |
|------|-------------|
| `src/solver/branch_and_bound.c` | Branch-and-bound implementation |
| `src/solver/branch_and_bound.h` | Branch-and-bound header |
| `src/solver/branch_and_cut.c` | Branch-and-cut implementation |
| `src/solver/branch_and_cut.h` | Branch-and-cut header |
| `src/solver/lagrangean.c` | Lagrangean relaxation implementation |
| `src/solver/lagrangean.h` | Lagrangean relaxation header |
| `src/solver/solver.c` | Solver factory (wires solvers to enum types) |
| `test/test_solver.c` | Tests for all solver types |
