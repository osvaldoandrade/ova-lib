#include "lagrangean.h"
#include "simplex.h"
#include "../matrix/matrix_internal.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define LR_MAX_ITER     200
#define LR_INT_TOL      1e-6
#define LR_STEP_TOL     1e-8
#define LR_INITIAL_STEP 2.0
#define LR_STEP_SHRINK  0.5

/**
 * @brief Check whether a rounded solution is feasible for the original problem.
 */
static int lr_check_feasibility(const double *solution, int num_vars,
                                double *const *constraint_data,
                                const double *bounds_data, int num_constraints) {
    for (int i = 0; i < num_constraints; i++) {
        double lhs = 0.0;
        for (int j = 0; j < num_vars; j++) {
            lhs += constraint_data[i][j] * solution[j];
        }
        if (lhs > bounds_data[i] + LR_INT_TOL) {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief Compute objective value for a given solution.
 */
static double lr_compute_objective(const double *solution, const double *obj_coeffs,
                                   int num_vars) {
    double val = 0.0;
    for (int j = 0; j < num_vars; j++) {
        val += obj_coeffs[j] * solution[j];
    }
    return val;
}

/**
 * @brief Lagrangean relaxation solver.
 *
 * Uses the LP relaxation solved by simplex as a basis. Applies subgradient
 * optimization to penalize constraint violations while pushing the LP solution
 * toward integrality. At each iteration, the LP solution is rounded and checked
 * for feasibility against the original constraints.
 */
int lagrangean_solver(lp_problem *prob, matrix **out_tableau) {
    if (!out_tableau) {
        return INFEASIBLE;
    }
    *out_tableau = NULL;

    lp_problem_impl *impl = lp_problem_impl_from_public(prob);
    if (!impl) {
        return INFEASIBLE;
    }

    matrix_impl *constraints = matrix_impl_from_matrix(impl->constraints);
    vector_impl *objective = vector_impl_from_vector(impl->objective);
    vector_impl *bounds = vector_impl_from_vector(impl->bounds);
    if (!constraints || !objective || !bounds) {
        return INFEASIBLE;
    }

    int num_vars = impl->variable_count;
    int num_constraints = impl->constraint_count;
    int is_max = (impl->type == PROBLEM_MAX) ? 1 : 0;

    if (num_constraints <= 0) {
        return INFEASIBLE;
    }

    /* Step 1: Solve the LP relaxation via simplex to get continuous optimum */
    solver *lp_solver = create_solver(SOLVER_SIMPLEX);
    if (!lp_solver) {
        return INFEASIBLE;
    }

    /* Clone the problem for LP solve */
    lp_problem *lp_prob = create_problem(num_vars, num_constraints);
    if (!lp_prob) {
        lp_solver->free(lp_solver);
        return INFEASIBLE;
    }

    double *obj_buf = (double *)malloc(sizeof(double) * (size_t)num_vars);
    if (!obj_buf) {
        lp_prob->free(lp_prob);
        lp_solver->free(lp_solver);
        return INFEASIBLE;
    }
    for (int j = 0; j < num_vars; j++) {
        obj_buf[j] = objective->data[j];
    }
    lp_prob->set_objective(lp_prob, obj_buf, impl->type);

    double *row_buf = (double *)malloc(sizeof(double) * (size_t)num_vars);
    if (!row_buf) {
        free(obj_buf);
        lp_prob->free(lp_prob);
        lp_solver->free(lp_solver);
        return INFEASIBLE;
    }
    for (int i = 0; i < num_constraints; i++) {
        for (int j = 0; j < num_vars; j++) {
            row_buf[j] = constraints->data[i][j];
        }
        lp_prob->add_constraint(lp_prob, row_buf, bounds->data[i]);
    }
    free(row_buf);

    matrix *lp_tableau = NULL;
    int lp_status = lp_solver->solve(lp_solver, lp_prob, &lp_tableau);
    if (lp_status != OPTIMAL) {
        free(obj_buf);
        if (lp_tableau) {
            lp_tableau->free(lp_tableau);
        }
        lp_solver->free(lp_solver);
        lp_prob->free(lp_prob);
        return lp_status;
    }

    lp_problem_impl *lp_impl = lp_problem_impl_from_public(lp_prob);
    double *lp_solution = lp_impl->solution;

    /* If LP is already integer, done */
    int all_integer = 1;
    for (int j = 0; j < num_vars; j++) {
        if (!is_integer(lp_solution[j])) {
            all_integer = 0;
            break;
        }
    }
    if (all_integer) {
        if (!impl->solution) {
            impl->solution = (double *)calloc((size_t)num_vars, sizeof(double));
        }
        if (impl->solution) {
            for (int j = 0; j < num_vars; j++) {
                impl->solution[j] = round(lp_solution[j]);
            }
        }
        impl->objective_value = lr_compute_objective(impl->solution, objective->data, num_vars);
        *out_tableau = lp_tableau;
        free(obj_buf);
        lp_solver->free(lp_solver);
        lp_prob->free(lp_prob);
        return OPTIMAL;
    }

    /* Step 2: Lagrangean subgradient optimization with rounding heuristic */
    double *lambda = (double *)calloc((size_t)num_constraints, sizeof(double));
    double *best_sol = (double *)calloc((size_t)num_vars, sizeof(double));
    double *rounded = (double *)calloc((size_t)num_vars, sizeof(double));
    double *modified_obj = (double *)malloc(sizeof(double) * (size_t)num_vars);
    double *subgradient = (double *)calloc((size_t)num_constraints, sizeof(double));
    if (!lambda || !best_sol || !rounded || !modified_obj || !subgradient) {
        free(lambda);
        free(best_sol);
        free(rounded);
        free(modified_obj);
        free(subgradient);
        free(obj_buf);
        if (lp_tableau) {
            lp_tableau->free(lp_tableau);
        }
        lp_solver->free(lp_solver);
        lp_prob->free(lp_prob);
        return INFEASIBLE;
    }

    double best_obj = is_max ? -DBL_MAX : DBL_MAX;
    double step_size = LR_INITIAL_STEP;
    int no_improve_count = 0;

    for (int iter = 0; iter < LR_MAX_ITER; iter++) {
        /* Build modified objective with Lagrangean penalties */
        for (int j = 0; j < num_vars; j++) {
            modified_obj[j] = objective->data[j];
            for (int i = 0; i < num_constraints; i++) {
                if (is_max) {
                    modified_obj[j] -= lambda[i] * constraints->data[i][j];
                } else {
                    modified_obj[j] += lambda[i] * constraints->data[i][j];
                }
            }
        }

        /* Solve LP with modified objective */
        lp_problem *sub = create_problem(num_vars, num_constraints);
        if (!sub) {
            break;
        }
        sub->set_objective(sub, modified_obj, impl->type);

        double *sub_row = (double *)malloc(sizeof(double) * (size_t)num_vars);
        if (!sub_row) {
            sub->free(sub);
            break;
        }
        for (int i = 0; i < num_constraints; i++) {
            for (int j = 0; j < num_vars; j++) {
                sub_row[j] = constraints->data[i][j];
            }
            sub->add_constraint(sub, sub_row, bounds->data[i]);
        }
        free(sub_row);

        solver *sub_solver = create_solver(SOLVER_SIMPLEX);
        if (!sub_solver) {
            sub->free(sub);
            break;
        }

        matrix *sub_tableau = NULL;
        int sub_status = sub_solver->solve(sub_solver, sub, &sub_tableau);

        if (sub_status != OPTIMAL) {
            if (sub_tableau) {
                sub_tableau->free(sub_tableau);
            }
            sub_solver->free(sub_solver);
            sub->free(sub);
            break;
        }

        lp_problem_impl *si = lp_problem_impl_from_public(sub);

        /* Round the LP solution to nearest integers */
        for (int j = 0; j < num_vars; j++) {
            rounded[j] = round(si->solution[j]);
            if (rounded[j] < 0.0) {
                rounded[j] = 0.0;
            }
        }

        /* Check feasibility of the rounded solution */
        if (lr_check_feasibility(rounded, num_vars, constraints->data,
                                 bounds->data, num_constraints)) {
            double rounded_obj = lr_compute_objective(rounded, objective->data, num_vars);
            int update = is_max ? (rounded_obj > best_obj) : (rounded_obj < best_obj);
            if (update) {
                best_obj = rounded_obj;
                memcpy(best_sol, rounded, sizeof(double) * (size_t)num_vars);
            }
        }

        /* Also try floor-based rounding */
        for (int j = 0; j < num_vars; j++) {
            rounded[j] = floor(si->solution[j]);
            if (rounded[j] < 0.0) {
                rounded[j] = 0.0;
            }
        }
        if (lr_check_feasibility(rounded, num_vars, constraints->data,
                                 bounds->data, num_constraints)) {
            double rounded_obj = lr_compute_objective(rounded, objective->data, num_vars);
            int update = is_max ? (rounded_obj > best_obj) : (rounded_obj < best_obj);
            if (update) {
                best_obj = rounded_obj;
                memcpy(best_sol, rounded, sizeof(double) * (size_t)num_vars);
            }
        }

        /* Compute Lagrangean bound */
        double lagrangean_bound = si->objective_value;
        for (int i = 0; i < num_constraints; i++) {
            lagrangean_bound += lambda[i] * bounds->data[i];
        }

        /* Track bound improvement for step size adjustment */
        int bound_improved = 0;
        if (iter == 0) {
            bound_improved = 1;
        } else if (is_max && lagrangean_bound < best_obj + LR_INT_TOL) {
            bound_improved = 1;
        } else if (!is_max && lagrangean_bound > best_obj - LR_INT_TOL) {
            bound_improved = 1;
        }

        if (!bound_improved) {
            no_improve_count++;
            if (no_improve_count >= 10) {
                step_size *= LR_STEP_SHRINK;
                no_improve_count = 0;
            }
        } else {
            no_improve_count = 0;
        }

        /* Compute subgradient: g_i = b_i - sum_j(a_ij * x_j) */
        double sg_norm_sq = 0.0;
        for (int i = 0; i < num_constraints; i++) {
            double lhs = 0.0;
            for (int j = 0; j < num_vars; j++) {
                lhs += constraints->data[i][j] * si->solution[j];
            }
            subgradient[i] = bounds->data[i] - lhs;
            sg_norm_sq += subgradient[i] * subgradient[i];
        }

        if (sub_tableau) {
            sub_tableau->free(sub_tableau);
        }
        sub_solver->free(sub_solver);
        sub->free(sub);

        /* Convergence check */
        if (step_size < LR_STEP_TOL || sg_norm_sq < LR_STEP_TOL) {
            break;
        }

        /* Update multipliers using subgradient step */
        double gap = fabs(lagrangean_bound) + 1.0;
        if ((is_max && best_obj > -DBL_MAX + 1.0) ||
            (!is_max && best_obj < DBL_MAX - 1.0)) {
            gap = fabs(lagrangean_bound - best_obj);
            if (gap < LR_STEP_TOL) {
                break; /* Converged */
            }
        }

        double t = step_size * gap / sg_norm_sq;

        for (int i = 0; i < num_constraints; i++) {
            if (is_max) {
                lambda[i] += t * subgradient[i];
            } else {
                lambda[i] -= t * subgradient[i];
            }
            if (lambda[i] < 0.0) {
                lambda[i] = 0.0;
            }
        }
    }

    free(lambda);
    free(rounded);
    free(modified_obj);
    free(subgradient);
    free(obj_buf);
    if (lp_tableau) {
        lp_tableau->free(lp_tableau);
    }
    lp_solver->free(lp_solver);
    lp_prob->free(lp_prob);

    /* Check if we found a feasible integer solution */
    if ((is_max && best_obj <= -DBL_MAX + 1.0) ||
        (!is_max && best_obj >= DBL_MAX - 1.0)) {
        free(best_sol);
        return INFEASIBLE;
    }

    /* Copy results */
    if (!impl->solution) {
        impl->solution = (double *)calloc((size_t)num_vars, sizeof(double));
    }
    if (!impl->solution) {
        free(best_sol);
        return INFEASIBLE;
    }

    memcpy(impl->solution, best_sol, sizeof(double) * (size_t)num_vars);
    impl->objective_value = best_obj;
    free(best_sol);

    *out_tableau = create_matrix(1, num_vars + 1);
    if (*out_tableau) {
        matrix_impl *tab = matrix_impl_from_matrix(*out_tableau);
        if (tab) {
            for (int i = 0; i < num_vars; i++) {
                tab->data[0][i] = impl->solution[i];
            }
            tab->data[0][num_vars] = impl->objective_value;
        }
    }

    return OPTIMAL;
}
