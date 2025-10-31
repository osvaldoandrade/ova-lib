# Linear Programming Solver

## Problem model
- `create_problem` provisions the constraint matrix (`numConstraints × numVariables`), objective vector, and bounds vector. Each allocation is validated; failures free all partially initialized components before returning NULL.
- Problems start with `type = PROBLEM_MAX`, `solution = NULL`, and `z_value = 0`. Function pointers expose `addConstraint`, `setObjective`, `setBounds`, and `print` for incremental configuration.
- `addConstraint` resizes the matrix by one row, copies the coefficients into the newly added row, grows the bounds vector, and stores the RHS. Constraints are interpreted as ≤ inequalities.
- `setObjective` copies coefficients verbatim and records whether the problem is a maximization or minimization.
- `setBounds` overwrites the RHS vector in place; callers must respect the current constraint count.

## Solver selection
- `create_solver` instantiates a solver wrapper with `solve` and `destroy` function pointers. Only `SOLVER_SIMPLEX` is implemented today; other enum values return stubs but keep binary compatibility for future algorithms.

## Simplex initialization
- `simplex_solver` validates that constraints, objective, and bounds are present before allocating a tableau with `rows + 1` rows and `cols + rows + 1` columns. The final column stores RHS values and the last row stores the objective.
- Constraint rows copy coefficients directly and append slack variables on the diagonal. Bounds populate the RHS column.
- The objective row stores the negated coefficients to model a maximization problem in canonical simplex form.
- `out_tableau` returns the allocated tableau to the caller, who assumes ownership and must call `matrix->destroy` when finished.

## Pivot strategy
- The solver scans the objective row from left to right and chooses the first negative coefficient as the entering variable (a Bland-like rule that mitigates cycling).
- The minimum ratio test divides each RHS by the pivot column entry for rows with positive pivot column entries, selecting the smallest ratio as the leaving variable. Lack of a valid pivot row signals an unbounded problem.
- Pivoting normalizes the pivot row by dividing by the pivot value, then subtracts multiples of the pivot row from every other row to zero the pivot column.

## Termination and reporting
- When no negative coefficients remain in the objective row, the solution is optimal. The solver reconstructs basic variable values by scanning each column for unit vectors and reading the associated RHS entry.
- `prob->solution` is allocated on demand and filled with the recovered basic variable values; nonbasic variables default to zero.
- `prob->z_value` records the objective value as stored in the tableau’s last row, last column (already negated back to the maximization value).
- The solver returns `OPTIMAL`, `UNBOUNDED`, or `INFEASIBLE` depending on validation and pivot outcomes.

## Utility hooks
- `is_feasible` evaluates whether a candidate solution satisfies all constraints; `improves_objective` compares objective deltas, and `is_integer` tests integrality using an epsilon threshold. These helpers are currently unused by the simplex loop but prepare for branch-and-bound and cutting-plane integrations.
