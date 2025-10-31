#include "solver.h"
#include <stdio.h>

#include <math.h>

#define FLOAT_TOL 0.001

int approx_equal(double a, double b) {
  return fabs(a - b) < FLOAT_TOL;
}

void test_simplex_solver1() {
  int numVariables = 2;
  int numConstraints = 3;
  lp_problem *problem = create_problem(numVariables, numConstraints);

  double objective_coeffs[] = {3, 5};
  problem->setObjective(problem, objective_coeffs, PROBLEM_MAX);

  double constraint1[] = {1, 2};
  double constraint2[] = {-3, 1};
  double constraint3[] = {1, -1};
  problem->addConstraint(problem, constraint1, 14);
  problem->addConstraint(problem, constraint2, 0);
  problem->addConstraint(problem, constraint3, 2);

  solver *mySolver = create_solver(SOLVER_SIMPLEX);
  matrix *solution;
  int result = mySolver->solve(problem, &solution);

  if (result == OPTIMAL) {
    printf("Optimal solution found.\n");
    printf("x = %f, y = %f, z = %f\n", problem->solution[0], problem->solution[1], problem->z_value);
    if (approx_equal(problem->solution[0], 6) && approx_equal(problem->solution[1], 4) && approx_equal(problem->z_value, 38)) {
      printf("Test passed. Correct solution (x, y, z) = (%f, %f, %f)\n", problem->solution[0], problem->solution[1], problem->z_value);
    } else {
      printf("Test failed. Incorrect solution (x, y, z) = (%f, %f, %f)\n", problem->solution[0], problem->solution[1], problem->z_value);
    }
  } else if (result == UNBOUNDED) {
    printf("Problem is unbounded.\n");
  } else {
    printf("Problem is infeasible or another error occurred.\n");
  }

  if (solution) {
    solution->destroy(solution);
  }
}

void test_solver_initializes_solution_vector() {
  int numVariables = 2;
  int numConstraints = 2;
  lp_problem *problem = create_problem(numVariables, numConstraints);

  double objective_coeffs[] = {1, 1};
  problem->setObjective(problem, objective_coeffs, PROBLEM_MAX);

  double constraint1[] = {1, 0};
  double constraint2[] = {0, 1};
  problem->addConstraint(problem, constraint1, 1);
  problem->addConstraint(problem, constraint2, 1);

  solver *mySolver = create_solver(SOLVER_SIMPLEX);
  matrix *solution = NULL;
  int result = mySolver->solve(problem, &solution);

  if (result == OPTIMAL && problem->solution != NULL) {
    printf("Solver initialized solution vector successfully. x1 = %f, x2 = %f, z = %f\n",
           problem->solution[0], problem->solution[1], problem->z_value);
  } else {
    printf("Solver failed to initialize solution vector.\n");
  }

  if (solution) {
    solution->destroy(solution);
  }
}

void test_simplex_solver2() {
  int numVariables = 3;
  int numConstraints = 3;
  lp_problem *problem = create_problem(numVariables, numConstraints);

  double objective_coeffs[] = {2, 3, 4};  // Novos coeficientes para a função objetivo
  problem->setObjective(problem, objective_coeffs, PROBLEM_MAX);

  double constraint1[] = {1, 1, 1};  // x + y + z <= 30
  double constraint2[] = {2, 2, 5};  // 2x + 2y + 5z <= 100
  double constraint3[] = {4, 1, 2};  // 4x + y + 2z <= 60
  problem->addConstraint(problem, constraint1, 30);
  problem->addConstraint(problem, constraint2, 100);
  problem->addConstraint(problem, constraint3, 60);

  solver *mySolver = create_solver(SOLVER_SIMPLEX);
  matrix *solution;
  int result = mySolver->solve(problem, &solution);

  if (result == OPTIMAL) {
    printf("Optimal solution found for alternative problem.\n");
    printf("x1 = %f, x2 = %f, x3 = %f, z = %f\n", problem->solution[0], problem->solution[1], problem->solution[2], problem->z_value);
  } else {
    printf("Problem is unbounded or infeasible.\n");
  }

  if (solution) {
    solution->destroy(solution);
  }
}

int main() {
  test_solver_initializes_solution_vector();
  test_simplex_solver1();
  test_simplex_solver2();
  return 0;
}
