#include "../../include/matrix.h"
#include "../../include/solver.h"
//#include "branch_cut.h"
//#include "branch_bound.h"
//#include "lagrange.h"
#include "simplex.h"
#include <math.h>

static void solver_add_constraint(lp_problem *self, double *coefficients, double bound);
static void solver_set_objective(lp_problem *self, double *coefficients, ProblemType type);
static void solver_set_bounds(lp_problem *self, double *bounds);
static void solver_print_problem(lp_problem *self);
static void solver_destroy(solver *s);

lp_problem *create_problem(int numVariables, int numConstraints) {
  lp_problem *problem = malloc(sizeof(lp_problem));
  if (!problem) {
    return NULL;  // Memory allocation failed
  }

  // Create and initialize the matrix and vectors with specified sizes
  problem->constraints = create_matrix(numConstraints, numVariables);
  problem->objective = create_vector(numVariables);
  problem->bounds = create_vector(numConstraints);

  // Check if any creations failed, and clean up if so
  if (!problem->constraints || !problem->objective || !problem->bounds) {
    if (problem->constraints)
      problem->constraints->destroy(problem->constraints);
    if (problem->objective)
      problem->objective->destroy(problem->objective);
    if (problem->bounds)
      problem->bounds->destroy(problem->bounds);
    free(problem);
    return NULL;
  }

  // Ensure optional fields are initialized to safe defaults
  problem->solution = NULL;
  problem->z_value = 0.0;
  problem->type = PROBLEM_MAX;

  // Assign utility functions
  problem->addConstraint = solver_add_constraint;
  problem->setObjective = solver_set_objective;
  problem->setBounds = solver_set_bounds;
  problem->print = solver_print_problem;

  return problem;
}

solver *create_solver(SolverType type) {
  solver *s = malloc(sizeof(solver));
  if (!s)
    return NULL;

  switch (type) {
  case SOLVER_SIMPLEX:s->solve = simplex_solver;
    break;
  case SOLVER_LAGRANGEAN_SIMPLEX: break; //s->solve = lagrange_solver;
    break;
  case SOLVER_BRANCH_AND_CUT: break; //s->solve = branch_cut_solver;
    break;
  case SOLVER_BRANCH_AND_BOUND: break; //s->solve = branch_bound_solver;
    break;
  default:free(s);
    return NULL;
  }

  s->destroy = solver_destroy;
  return s;
}

static void solver_add_constraint(lp_problem *self, double *coefficients, double bound) {
  if (self == NULL || coefficients == NULL)
    return;

  int currentRows = self->constraints->rows;

  // Resize the matrix to accommodate one more constraint
  if (self->constraints->resize(self->constraints, currentRows + 1,
                                self->constraints->cols) != 0) {
    return;
  }

  // Set the new constraint's coefficients
  for (int i = 0; i < self->constraints->cols; i++) {
    self->constraints->data[currentRows][i] = coefficients[i];
  }

  // Resize the bounds vector and set the new bound
  self->bounds->resize(self->bounds, currentRows + 1);
  self->bounds->data[currentRows] = bound;
}

static void solver_set_objective(lp_problem *self, double *coefficients, ProblemType type) {
  if (self == NULL || coefficients == NULL)
    return;

  for (int i = 0; i < self->objective->size; i++) {
    self->objective->data[i] = coefficients[i];
  }
  self->type = type;
}

static void solver_set_bounds(lp_problem *self, double *bounds) {
  if (self == NULL || bounds == NULL)
    return;

  for (int i = 0; i < self->bounds->size; i++) {
    self->bounds->data[i] = bounds[i];
  }
}

static void solver_print_problem(lp_problem *self) {
  if (self == NULL)
    return;

  printf("Objective Function (%s):\n", (self->type == PROBLEM_MAX ? "Maximize" : "Minimize"));
  for (int i = 0; i < self->objective->size; i++) {
    printf("%f ", self->objective->data[i]);
  }
  printf("\n\nConstraints (<=):\n");
  for (int i = 0; i < self->constraints->rows; i++) {
    for (int j = 0; j < self->constraints->cols; j++) {
      printf("%f*x%d ", self->constraints->data[i][j], j + 1);
      if (j < self->constraints->cols - 1)
        printf("+ ");
    }
    printf("<= %f\n", self->bounds->data[i]);
  }
  printf("\n");
}

static void solver_destroy(solver *s) {
  if (s == NULL) {
    return;
  }
  free(s);
}

static int is_feasible(lp_problem *prob, vector *solution) {
  for (int i = 0; i < prob->constraints->rows; i++) {
    double sum = 0;
    for (int j = 0; j < prob->constraints->cols; j++) {
      sum += prob->constraints->data[i][j] * solution->data[j];
    }
    if (sum > prob->bounds->data[i])
      return 0;  // Constraint violated
  }
  return 1;
}

int is_integer(double value) {
  return fabs(value - floor(value)) < 1e-6;  // Using a small epsilon to handle floating-point precision issues
}

int improves_objective(lp_problem *prob, vector *solution, double old_value, int index) {
  double new_obj = 0, old_obj = 0;
  for (int i = 0; i < solution->size; i++) {
    new_obj += prob->objective->data[i] * solution->data[i];
    old_obj += prob->objective->data[i] * ((i == index) ? old_value : solution->data[i]);
  }
  // Compare strings correctly
  return (prob->type == PROBLEM_MAX) ? (new_obj > old_obj) : (new_obj < old_obj);
}


