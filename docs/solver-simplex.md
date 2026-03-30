# Linear Programming Solver

The solver layer in `solver.h` exposes 4 enum values, but the current implementation only supports `SOLVER_SIMPLEX`. The other enum values are declarations in the header, not working algorithm selections in the current source tree.

## Problem Construction

`create_problem(numVariables, numConstraints)` allocates 3 numeric objects:

- one constraint matrix with `numConstraints × numVariables`
- one objective vector with `numVariables` slots
- one bounds vector with `numConstraints` slots

If `numConstraints < 1`, the implementation raises the capacity to `1`. The problem tracks `constraint_count` separately from `constraint_capacity`, so you can add constraints incrementally through `addConstraint`.

## Mutators

`setObjective` copies the objective coefficients and stores `PROBLEM_MAX` or `PROBLEM_MIN`.

`addConstraint` appends one `<=` row. If the current capacity is full, it doubles the backing matrix rows and the bounds vector length before copying the new coefficients and bound.

`setBounds` overwrites the active bounds in place. `print` renders the objective and the active `<=` constraints.

## Solving

`create_solver(SOLVER_SIMPLEX)` installs the simplex implementation in `solve`. The solve callback writes a tableau matrix through `matrix **out_tableau` and returns one of 3 status codes from the header:

| Status | Value |
| --- | --- |
| `OPTIMAL` | `0` |
| `UNBOUNDED` | `-1` |
| `INFEASIBLE` | `-2` |

On `OPTIMAL`, the solver allocates `problem->solution`, fills the basic variable values, and stores the objective value in `problem->z_value`.

## Cleanup

Destroy the tableau with `matrix->destroy`. Destroy the solver with `solver->destroy`. Destroy the problem with `destroy_problem`.

The solver object owns only its own wrapper. The problem object owns the constraint matrix, objective vector, bounds vector, and solution array.
