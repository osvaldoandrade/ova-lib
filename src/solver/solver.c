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
static int solver_ensure_constraint_capacity(lp_problem *self, int min_capacity);

lp_problem *create_problem(int numVariables, int numConstraints) {
  lp_problem *problem = malloc(sizeof(lp_problem));
  if (!problem) {
    return NULL;  // Memory allocation failed
  }

  if (numVariables <= 0) {
    free(problem);
    return NULL;
  }

  if (numConstraints < 1) {
    numConstraints = 1;
  }

  // Keep capacity separate from the number of populated constraints.
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
  problem->constraint_count = 0;
  problem->constraint_capacity = numConstraints;

  // Assign utility functions
  problem->addConstraint = solver_add_constraint;
  problem->setObjective = solver_set_objective;
  problem->setBounds = solver_set_bounds;
  problem->print = solver_print_problem;

  return problem;
}

void destroy_problem(lp_problem *problem) {
  if (problem == NULL) {
    return;
  }

  if (problem->constraints) {
    problem->constraints->destroy(problem->constraints);
  }
  if (problem->objective) {
    problem->objective->destroy(problem->objective);
  }
  if (problem->bounds) {
    problem->bounds->destroy(problem->bounds);
  }
  free(problem->solution);
  free(problem);
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

  if (solver_ensure_constraint_capacity(self, self->constraint_count + 1) != 0) {
    return;
  }

  int row = self->constraint_count;
  for (int i = 0; i < self->constraints->cols; i++) {
    self->constraints->data[row][i] = coefficients[i];
  }

  self->bounds->data[row] = bound;
  self->constraint_count++;
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

  for (int i = 0; i < self->constraint_count; i++) {
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
  for (int i = 0; i < self->constraint_count; i++) {
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

static int solver_ensure_constraint_capacity(lp_problem *self, int min_capacity) {
  if (self == NULL || min_capacity <= self->constraint_capacity) {
    return 0;
  }

  int new_capacity = self->constraint_capacity;
  while (new_capacity < min_capacity) {
    new_capacity *= 2;
  }

  if (self->constraints->resize(self->constraints, new_capacity, self->constraints->cols) != 0) {
    return -1;
  }

  self->bounds->resize(self->bounds, new_capacity);
  if (self->bounds->size < new_capacity) {
    return -1;
  }

  self->constraint_capacity = new_capacity;
  return 0;
}

static int is_feasible(lp_problem *prob, vector *solution) {
  for (int i = 0; i < prob->constraint_count; i++) {
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

