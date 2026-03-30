#ifndef SOLVER_H
#define SOLVER_H

/**
 * @file solver.h
 * @brief Linear programming solver framework.
 *
 * Provides a solver interface for linear programming problems with support for
 * the simplex method and related algorithms.
 */

#include "types.h"
#include "matrix.h"

#define OPTIMAL (0)      /**< Solver result: optimal solution found. */
#define UNBOUNDED (-1)   /**< Solver result: problem is unbounded. */
#define INFEASIBLE (-2)  /**< Solver result: problem is infeasible. */

/**
 * @brief Available solver algorithm types.
 */
typedef enum {
  SOLVER_BRANCH_AND_CUT,     /**< Branch and cut algorithm. */
  SOLVER_BRANCH_AND_BOUND,   /**< Branch and bound algorithm. */
  SOLVER_LAGRANGEAN_SIMPLEX, /**< Lagrangean relaxation with simplex. */
  SOLVER_SIMPLEX             /**< Simplex method. */
} SolverType;

/**
 * @brief Optimization direction for a linear programming problem.
 */
typedef enum {
  PROBLEM_MAX, /**< Maximize the objective function. */
  PROBLEM_MIN, /**< Minimize the objective function. */
} ProblemType;

/**
 * @brief Represents a linear programming problem.
 */
typedef struct lp_problem {
  matrix* constraints;       /**< Constraint coefficient matrix. */
  vector* objective;         /**< Objective function coefficients. */
  vector* bounds;            /**< Right-hand side bounds for constraints. */
  ProblemType type;          /**< Optimization direction. */
  int constraint_count;      /**< Current number of constraints. */
  int constraint_capacity;   /**< Allocated capacity for constraints. */

  double* solution;          /**< Array storing solution values for decision variables. */
  double z_value;            /**< Objective function value at the solution. */

  /**
   * @brief Adds a constraint to the problem.
   *
   * @param self The problem.
   * @param coefficients Array of constraint coefficients.
   * @param bound Right-hand side bound for the constraint.
   */
  void (*addConstraint)(struct lp_problem* self, double* coefficients, double bound);

  /**
   * @brief Sets the objective function.
   *
   * @param self The problem.
   * @param coefficients Array of objective function coefficients.
   * @param type Optimization direction (PROBLEM_MAX or PROBLEM_MIN).
   */
  void (*setObjective)(struct lp_problem* self, double* coefficients, ProblemType type);

  /**
   * @brief Sets the constraint bounds.
   *
   * @param self The problem.
   * @param bounds Array of right-hand side bounds.
   */
  void (*setBounds)(struct lp_problem* self, double* bounds);

  /**
   * @brief Prints the problem to standard output.
   *
   * @param self The problem.
   */
  void (*print)(struct lp_problem* self);
} lp_problem;

/**
 * @brief Function pointer type for solver algorithms.
 *
 * @param problem The linear programming problem to solve.
 * @param out_tableau Pointer that receives the final simplex tableau.
 * @return OPTIMAL, UNBOUNDED, or INFEASIBLE.
 */
typedef int (*solver_func)(lp_problem *problem, matrix **out_tableau);

/**
 * @brief Solver structure encapsulating an LP algorithm.
 */
typedef struct solver {
  SolverType type;     /**< Algorithm type. */
  solver_func solve;   /**< Function pointer to the solver algorithm. */

  /**
   * @brief Frees all memory associated with the solver.
   *
   * @param self The solver to destroy.
   */
  void (*destroy)(struct solver* self);
} solver;

/**
 * @brief Creates a solver for the specified algorithm type.
 *
 * @param type The solver algorithm to use (e.g., SOLVER_SIMPLEX).
 * @return A pointer to the newly created solver, or NULL on failure.
 */
solver* create_solver(SolverType type);

/**
 * @brief Creates a new linear programming problem.
 *
 * @param numVariables The number of decision variables.
 * @param numConstraints The initial number of constraints.
 * @return A pointer to the newly created problem, or NULL on allocation failure.
 */
lp_problem* create_problem(int numVariables, int numConstraints);

/**
 * @brief Frees all memory associated with a linear programming problem.
 *
 * @param problem The problem to destroy.
 */
void destroy_problem(lp_problem *problem);

/**
 * @brief Checks whether a given solution is feasible for a matrix programming problem.
 *
 * This function checks whether the given solution satisfies all the constraints of the
 * matrix programming problem defined by `prob`.
 *
 * @param prob The matrix programming problem.
 * @param solution The solution vector to be checked.
 * @return 1 if the solution is feasible, 0 otherwise.
 */
int is_feasible(lp_problem *prob, vector *solution);

/**
 * @brief Determines whether a given solution improves the objective value.
 *
 * @param prob      The matrix programming problem.
 * @param solution  The solution vector.
 * @param old_value The previous value at the specified index.
 * @param index     The index to update in the solution vector.
 * @return          Returns 1 if the solution improves the objective value, 0 otherwise.
 */
int improves_objective(lp_problem *prob, vector *solution, double old_value, int index);

/**
 * @brief Checks whether a floating-point value is an integer (within tolerance).
 *
 * @param value The value to check.
 * @return 1 if @p value is an integer, 0 otherwise.
 */
int is_integer(double value);

#endif // SOLVER_H
