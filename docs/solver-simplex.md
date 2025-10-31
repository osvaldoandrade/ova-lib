# Linear Programming Solver

## Problem model
`create_problem` provisions the constraint matrix with dimensions `numConstraints × numVariables`, the objective vector, and the bounds vector. Every allocation is validated, and failures trigger cleanup of partially initialized components before returning NULL. Newly created problems default to `type = PROBLEM_MAX`, `solution = NULL`, and `z_value = 0`. Function pointers expose `addConstraint`, `setObjective`, `setBounds`, and `print`, enabling incremental configuration. `addConstraint` grows the matrix by one row, copies the coefficients, expands the bounds vector, and stores the right-hand side while interpreting constraints as ≤ inequalities. `setObjective` copies coefficients verbatim and records whether the problem targets maximization or minimization. `setBounds` overwrites the RHS vector in place; callers must respect the active constraint count.

## Solver selection
`create_solver` instantiates a solver wrapper that wires `solve` and `destroy`. Only `SOLVER_SIMPLEX` is implemented today, while other enum values return stubs to preserve binary compatibility for future algorithms.

## Simplex initialization
`simplex_solver` verifies that constraints, objective coefficients, and bounds exist before allocating a tableau sized to `rows + 1` by `cols + rows + 1`. Constraint rows copy coefficients directly and append slack variables along the diagonal. Bounds populate the final column, and the last row stores the objective with negated coefficients to model maximization in canonical simplex form. Ownership of the tableau transfers to the caller through `out_tableau`, which must eventually invoke `matrix->destroy`.

## Pivot strategy
The solver scans the objective row from left to right and chooses the first negative coefficient as the entering variable, following a Bland-style rule that mitigates cycling. The minimum ratio test divides each right-hand side by the corresponding pivot column entry for rows with positive entries, selecting the smallest ratio as the leaving variable. When no valid pivot row exists, the problem is marked unbounded. Pivoting normalizes the chosen row by dividing by the pivot value, then subtracts multiples of that row from every other row to zero the pivot column.

## Termination and reporting
When the objective row contains no negative coefficients, the solution is optimal. The solver reconstructs basic variable values by scanning each column for unit vectors and reading the associated right-hand-side entry. `prob->solution` allocates storage on demand and fills it with recovered basic variable values while leaving nonbasic variables at zero. `prob->z_value` records the objective as the negated value stored in the tableau’s last row and column. The solver reports `OPTIMAL`, `UNBOUNDED`, or `INFEASIBLE` depending on validation and pivot outcomes.

## Utility hooks
Helper functions such as `is_feasible`, `improves_objective`, and `is_integer` assess constraints, objective deltas, and integrality using an epsilon threshold. They remain unused by the simplex loop today but lay groundwork for branch-and-bound or cutting-plane integrations.
