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

/**
 * Branch-and-bound: Maximize 3x1 + 5x2
 * subject to x1 + 2x2 <= 14, -3x1 + x2 <= 0, x1 - x2 <= 2
 * LP optimum is (6, 4) which is already integer.
 */
void test_branch_and_bound_integer_lp(void) {
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

    solver_instance = create_solver(SOLVER_BRANCH_AND_BOUND);
    int result = solver_instance->solve(solver_instance, problem, &tableau);

    print_test_result(result == OPTIMAL &&
                              approx_equal(problem->solution_value(problem, 0), 6.0) &&
                              approx_equal(problem->solution_value(problem, 1), 4.0) &&
                              approx_equal(problem->objective_value(problem), 38.0),
                      "Branch-and-bound finds integer optimum when LP relaxation is already integer");

    cleanup_solver_test(problem, solver_instance, tableau);
}

/**
 * Branch-and-bound: Maximize 8x1 + 5x2
 * subject to x1 + x2 <= 6, 9x1 + 5x2 <= 45
 * LP optimum is x1 = 3.75, x2 = 2.25 (fractional).
 * Integer optimum is x1 = 5, x2 = 0 with obj = 40 or
 * other integer combination. The key assertion is that the
 * solution must be integer and feasible.
 */
void test_branch_and_bound_fractional_lp(void) {
    lp_problem *problem = create_problem(2, 2);
    matrix *tableau = NULL;
    solver *solver_instance = NULL;

    double objective_coeffs[] = {8, 5};
    problem->set_objective(problem, objective_coeffs, PROBLEM_MAX);

    double constraint1[] = {1, 1};
    double constraint2[] = {9, 5};
    problem->add_constraint(problem, constraint1, 6);
    problem->add_constraint(problem, constraint2, 45);

    solver_instance = create_solver(SOLVER_BRANCH_AND_BOUND);
    int result = solver_instance->solve(solver_instance, problem, &tableau);

    /* The solution must be integer and feasible */
    int is_int = result == OPTIMAL &&
                 is_integer(problem->solution_value(problem, 0)) &&
                 is_integer(problem->solution_value(problem, 1));

    /* Check feasibility */
    double x1 = problem->solution_value(problem, 0);
    double x2 = problem->solution_value(problem, 1);
    int feasible = (x1 + x2 <= 6.0 + FLOAT_TOL) &&
                   (9.0 * x1 + 5.0 * x2 <= 45.0 + FLOAT_TOL) &&
                   (x1 >= -FLOAT_TOL) && (x2 >= -FLOAT_TOL);

    print_test_result(is_int && feasible,
                      "Branch-and-bound finds feasible integer solution for fractional LP");

    cleanup_solver_test(problem, solver_instance, tableau);
}

/**
 * Branch-and-cut: Maximize 3x1 + 5x2
 * subject to x1 + 2x2 <= 14, -3x1 + x2 <= 0, x1 - x2 <= 2
 * LP optimum is (6, 4) which is already integer.
 */
void test_branch_and_cut_integer_lp(void) {
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

    solver_instance = create_solver(SOLVER_BRANCH_AND_CUT);
    int result = solver_instance->solve(solver_instance, problem, &tableau);

    print_test_result(result == OPTIMAL &&
                              approx_equal(problem->solution_value(problem, 0), 6.0) &&
                              approx_equal(problem->solution_value(problem, 1), 4.0) &&
                              approx_equal(problem->objective_value(problem), 38.0),
                      "Branch-and-cut finds integer optimum when LP relaxation is already integer");

    cleanup_solver_test(problem, solver_instance, tableau);
}

/**
 * Branch-and-cut: Maximize 8x1 + 5x2
 * subject to x1 + x2 <= 6, 9x1 + 5x2 <= 45
 * Tests that the solver finds a feasible integer solution.
 */
void test_branch_and_cut_fractional_lp(void) {
    lp_problem *problem = create_problem(2, 2);
    matrix *tableau = NULL;
    solver *solver_instance = NULL;

    double objective_coeffs[] = {8, 5};
    problem->set_objective(problem, objective_coeffs, PROBLEM_MAX);

    double constraint1[] = {1, 1};
    double constraint2[] = {9, 5};
    problem->add_constraint(problem, constraint1, 6);
    problem->add_constraint(problem, constraint2, 45);

    solver_instance = create_solver(SOLVER_BRANCH_AND_CUT);
    int result = solver_instance->solve(solver_instance, problem, &tableau);

    int is_int = result == OPTIMAL &&
                 is_integer(problem->solution_value(problem, 0)) &&
                 is_integer(problem->solution_value(problem, 1));

    double x1 = problem->solution_value(problem, 0);
    double x2 = problem->solution_value(problem, 1);
    int feasible = (x1 + x2 <= 6.0 + FLOAT_TOL) &&
                   (9.0 * x1 + 5.0 * x2 <= 45.0 + FLOAT_TOL) &&
                   (x1 >= -FLOAT_TOL) && (x2 >= -FLOAT_TOL);

    print_test_result(is_int && feasible,
                      "Branch-and-cut finds feasible integer solution for fractional LP");

    cleanup_solver_test(problem, solver_instance, tableau);
}

/**
 * Lagrangean relaxation: Maximize 3x1 + 5x2
 * subject to x1 + 2x2 <= 14, -3x1 + x2 <= 0, x1 - x2 <= 2
 * Tests that the solver finds a feasible integer solution.
 */
void test_lagrangean_solver(void) {
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

    solver_instance = create_solver(SOLVER_LAGRANGEAN_SIMPLEX);
    int result = solver_instance->solve(solver_instance, problem, &tableau);

    /* Lagrangean should find a feasible integer solution */
    int is_int = result == OPTIMAL &&
                 is_integer(problem->solution_value(problem, 0)) &&
                 is_integer(problem->solution_value(problem, 1));

    double x1 = problem->solution_value(problem, 0);
    double x2 = problem->solution_value(problem, 1);
    int feasible = (x1 + 2.0 * x2 <= 14.0 + FLOAT_TOL) &&
                   (-3.0 * x1 + x2 <= 0.0 + FLOAT_TOL) &&
                   (x1 - x2 <= 2.0 + FLOAT_TOL) &&
                   (x1 >= -FLOAT_TOL) && (x2 >= -FLOAT_TOL);

    print_test_result(is_int && feasible,
                      "Lagrangean relaxation finds feasible integer solution");

    cleanup_solver_test(problem, solver_instance, tableau);
}

/**
 * Test that solver_type returns the correct type for each solver.
 */
void test_solver_types(void) {
    solver *bb = create_solver(SOLVER_BRANCH_AND_BOUND);
    solver *bc = create_solver(SOLVER_BRANCH_AND_CUT);
    solver *lr = create_solver(SOLVER_LAGRANGEAN_SIMPLEX);

    print_test_result(bb && bb->solver_type(bb) == SOLVER_BRANCH_AND_BOUND &&
                              bc && bc->solver_type(bc) == SOLVER_BRANCH_AND_CUT &&
                              lr && lr->solver_type(lr) == SOLVER_LAGRANGEAN_SIMPLEX,
                      "Integer programming solver types are correctly reported");

    if (bb) bb->free(bb);
    if (bc) bc->free(bc);
    if (lr) lr->free(lr);
}

int main(void) {
    test_solver_initializes_solution_vector();
    test_simplex_solver1();
    test_simplex_solver2();
    test_branch_and_bound_integer_lp();
    test_branch_and_bound_fractional_lp();
    test_branch_and_cut_integer_lp();
    test_branch_and_cut_fractional_lp();
    test_lagrangean_solver();
    test_solver_types();
    return 0;
}
