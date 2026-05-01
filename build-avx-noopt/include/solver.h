#ifndef SOLVER_H
#define SOLVER_H

#include "matrix.h"
#include "types.h"

#define OPTIMAL (0)
#define UNBOUNDED (-1)
#define INFEASIBLE (-2)

typedef enum {
    SOLVER_BRANCH_AND_CUT,
    SOLVER_BRANCH_AND_BOUND,
    SOLVER_LAGRANGEAN_SIMPLEX,
    SOLVER_SIMPLEX
} SolverType;

typedef enum {
    PROBLEM_MAX,
    PROBLEM_MIN
} ProblemType;

/**
 * @brief Public linear-programming problem object.
 *
 * Concrete storage details live in @p impl.
 */
typedef struct lp_problem {
    void *impl;
    void *user_data; /**< User-provided context pointer. */

    /**
     * @brief Add a new constraint row.
     *
     * @param self Problem instance.
     * @param coefficients Dense coefficient array with one entry per variable.
     * @param bound Right-hand-side bound for the row.
     * @return 0 on success, -1 on failure.
     */
    int (*add_constraint)(struct lp_problem *self, const double *coefficients, double bound);

    /**
     * @brief Set the objective coefficients and optimization direction.
     *
     * @param self Problem instance.
     * @param coefficients Dense coefficient array with one entry per variable.
     * @param type Optimization direction.
     * @return 0 on success, -1 on failure.
     */
    int (*set_objective)(struct lp_problem *self, const double *coefficients, ProblemType type);

    /**
     * @brief Replace the current constraint bounds.
     *
     * @param self Problem instance.
     * @param bounds Dense bound array with one entry per populated constraint.
     * @return 0 on success, -1 on failure.
     */
    int (*set_bounds)(struct lp_problem *self, const double *bounds);

    /**
     * @brief Check whether a candidate solution is feasible.
     *
     * @param self Problem instance.
     * @param solution Candidate solution vector.
     * @return 1 when feasible, 0 otherwise.
     */
    int (*is_feasible)(struct lp_problem *self, vector *solution);

    /**
     * @brief Check whether a candidate value improves the objective.
     *
     * @param self Problem instance.
     * @param solution Candidate solution vector.
     * @param old_value Previous value at @p index.
     * @param index Index to compare.
     * @return 1 when the objective improves, 0 otherwise.
     */
    int (*improves_objective)(struct lp_problem *self, vector *solution, double old_value, int index);

    /**
     * @brief Return the variable count.
     *
     * @param self Problem instance.
     * @return Number of variables.
     */
    int (*variable_count)(const struct lp_problem *self);

    /**
     * @brief Return the number of populated constraints.
     *
     * @param self Problem instance.
     * @return Number of populated constraint rows.
     */
    int (*constraint_count)(const struct lp_problem *self);

    /**
     * @brief Return one value from the latest solver solution vector.
     *
     * @param self Problem instance.
     * @param index Zero-based variable index.
     * @return Solution value, or 0.0 when unavailable / out of bounds.
     */
    double (*solution_value)(const struct lp_problem *self, int index);

    /**
     * @brief Return the latest objective value computed by the solver.
     *
     * @param self Problem instance.
     * @return Objective value from the latest solve.
     */
    double (*objective_value)(const struct lp_problem *self);

    /**
     * @brief Return the current optimization direction.
     *
     * @param self Problem instance.
     * @return Current problem type.
     */
    ProblemType (*problem_type)(const struct lp_problem *self);

    /**
     * @brief Print the problem definition to stdout.
     *
     * @param self Problem instance.
     */
    void (*print)(struct lp_problem *self);

    /**
     * @brief Release the problem and its internal allocations.
     *
     * @param self Problem instance.
     */
    void (*free)(struct lp_problem *self);
} lp_problem;

/**
 * @brief Public solver object.
 *
 * Concrete storage details live in @p impl.
 */
typedef struct solver {
    void *impl;
    void *user_data; /**< User-provided context pointer. */

    /**
     * @brief Solve a problem and optionally return the final tableau.
     *
     * @param self Solver instance.
     * @param problem Problem instance to solve.
     * @param out_tableau Optional output for the final simplex tableau.
     * @return OPTIMAL, UNBOUNDED, or INFEASIBLE.
     */
    int (*solve)(struct solver *self, lp_problem *problem, matrix **out_tableau);

    /**
     * @brief Return the configured solver type.
     *
     * @param self Solver instance.
     * @return Solver backend type.
     */
    SolverType (*solver_type)(const struct solver *self);

    /**
     * @brief Release the solver and its internal allocations.
     *
     * @param self Solver instance.
     */
    void (*free)(struct solver *self);
} solver;

solver *create_solver(SolverType type);
lp_problem *create_problem(int num_variables, int num_constraints);
int is_integer(double value);

#endif // SOLVER_H
