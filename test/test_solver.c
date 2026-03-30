#include "../include/solver.h"
#include "base_test.h"

#include <math.h>

#define FLOAT_TOL 0.001

static int approx_equal(double a, double b) {
    return fabs(a - b) < FLOAT_TOL;
}

static void cleanup_solver_test(lp_problem *problem, solver *solver_instance, matrix *tableau) {
    if (tableau) {
        tableau->free(tableau);
    }
    if (solver_instance) {
        solver_instance->free(solver_instance);
    }
    if (problem) {
        problem->free(problem);
    }
}

void test_simplex_solver1(void) {
    lp_problem *problem = create_problem(2, 3);
    matrix *tableau = NULL;
    solver *solver_instance = NULL;

    double objective_coeffs[] = {3, 5};
    problem->set_objective(problem, objective_coeffs, PROBLEM_MAX);

    double constraint1[] = {1, 2};
    double constraint2[] = {-3, 1};
    double constraint3[] = {1, -1};
    problem->add_constraint(problem, constraint1, 14);
    problem->add_constraint(problem, constraint2, 0);
    problem->add_constraint(problem, constraint3, 2);

    solver_instance = create_solver(SOLVER_SIMPLEX);
    int result = solver_instance->solve(solver_instance, problem, &tableau);

    print_test_result(result == OPTIMAL &&
                              approx_equal(problem->solution_value(problem, 0), 6.0) &&
                              approx_equal(problem->solution_value(problem, 1), 4.0) &&
                              approx_equal(problem->objective_value(problem), 38.0),
                      "Simplex solver finds the expected optimum for the 2-variable problem");

    cleanup_solver_test(problem, solver_instance, tableau);
}

void test_solver_initializes_solution_vector(void) {
    lp_problem *problem = create_problem(2, 2);
    matrix *tableau = NULL;
    solver *solver_instance = NULL;

    double objective_coeffs[] = {1, 1};
    problem->set_objective(problem, objective_coeffs, PROBLEM_MAX);

    double constraint1[] = {1, 0};
    double constraint2[] = {0, 1};
    problem->add_constraint(problem, constraint1, 1);
    problem->add_constraint(problem, constraint2, 1);

    solver_instance = create_solver(SOLVER_SIMPLEX);
    int result = solver_instance->solve(solver_instance, problem, &tableau);

    print_test_result(result == OPTIMAL &&
                              approx_equal(problem->solution_value(problem, 0), 1.0) &&
                              approx_equal(problem->solution_value(problem, 1), 1.0) &&
                              approx_equal(problem->objective_value(problem), 2.0),
                      "Solver stores the final solution vector behind accessors");

    cleanup_solver_test(problem, solver_instance, tableau);
}

void test_simplex_solver2(void) {
    lp_problem *problem = create_problem(3, 3);
    matrix *tableau = NULL;
    solver *solver_instance = NULL;

    double objective_coeffs[] = {2, 3, 4};
    problem->set_objective(problem, objective_coeffs, PROBLEM_MAX);

    double constraint1[] = {1, 1, 1};
    double constraint2[] = {2, 2, 5};
    double constraint3[] = {4, 1, 2};
    problem->add_constraint(problem, constraint1, 30);
    problem->add_constraint(problem, constraint2, 100);
    problem->add_constraint(problem, constraint3, 60);

    solver_instance = create_solver(SOLVER_SIMPLEX);
    int result = solver_instance->solve(solver_instance, problem, &tableau);

    print_test_result(result == OPTIMAL &&
                              approx_equal(problem->solution_value(problem, 0), 0.0) &&
                              approx_equal(problem->solution_value(problem, 1), 50.0 / 3.0) &&
                              approx_equal(problem->solution_value(problem, 2), 40.0 / 3.0) &&
                              approx_equal(problem->objective_value(problem), 310.0 / 3.0),
                      "Simplex solver finds the expected optimum for the 3-variable problem");

    cleanup_solver_test(problem, solver_instance, tableau);
}

int main(void) {
    test_solver_initializes_solution_vector();
    test_simplex_solver1();
    test_simplex_solver2();
    return 0;
}
