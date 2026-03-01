#ifndef SOLVER_H
#define SOLVER_H

#include "types.h"
#include "matrix.h"

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
  PROBLEM_MIN,
} ProblemType;

typedef struct lp_problem {
  matrix* constraints;
  vector* objective;
  vector* bounds;
  ProblemType type;

  double* solution; // Array to store solution values for X1, X2, etc.
  double z_value;   // Store the objective function value

  void (*addConstraint)(struct lp_problem* self, double* coefficients, double bound);
  void (*setObjective)(struct lp_problem* self, double* coefficients, ProblemType type);
  void (*setBounds)(struct lp_problem* self, double* bounds);
  void (*print)(struct lp_problem* self);
} lp_problem;

typedef int (*solver_func)(lp_problem *problem, matrix **out_tableau);

typedef struct solver {
  SolverType type;
  solver_func solve;
  void (*destroy)(struct solver* self);
} solver;

// Function to create a solver based on the specified type
solver* create_solver(SolverType type);

// Factory method for creating an LP problem
lp_problem* create_problem(int numVariables, int numConstraints);

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
static int is_feasible(lp_problem *prob, vector *solution);

/**
 * Determines whether a given solution improves the objective value of a matrix programming problem.
 *
 * @param prob      The matrix programming problem.
 * @param solution  The solution vector.
 * @param old_value The previous value at the specified index.
 * @param index     The index to update in the solution vector.
 * @return          Returns 1 if the solution improves the objective value, 0 otherwise.
 */
int improves_objective(lp_problem *prob, vector *solution, double old_value, int index);

int is_integer(double value);

#endif // SOLVER_H
