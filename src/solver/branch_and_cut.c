#include "branch_and_cut.h"
#include "simplex.h"
#include "../matrix/matrix_internal.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define BC_MAX_NODES      10000
#define BC_MAX_CUTS       10
#define BC_INT_TOL        1e-6
#define BC_COEFF_LIMIT    1e12

/**
 * @brief Clone an lp_problem including all constraints, objective, and bounds.
 */
static lp_problem *bc_clone_problem(lp_problem *src) {
    lp_problem_impl *si = lp_problem_impl_from_public(src);
    if (!si) {
        return NULL;
    }

    matrix_impl *sc = matrix_impl_from_matrix(si->constraints);
    vector_impl *so = vector_impl_from_vector(si->objective);
    vector_impl *sb = vector_impl_from_vector(si->bounds);
    if (!sc || !so || !sb) {
        return NULL;
    }

    lp_problem *dst = create_problem(si->variable_count,
                                     si->constraint_count > 0 ? si->constraint_count + 2 : 2);
    if (!dst) {
        return NULL;
    }

    lp_problem_impl *di = lp_problem_impl_from_public(dst);
    if (!di) {
        dst->free(dst);
        return NULL;
    }

    double *obj_buf = (double *)malloc(sizeof(double) * (size_t)si->variable_count);
    if (!obj_buf) {
        dst->free(dst);
        return NULL;
    }
    for (int i = 0; i < so->size; i++) {
        obj_buf[i] = so->data[i];
    }
    dst->set_objective(dst, obj_buf, si->type);
    free(obj_buf);

    double *row_buf = (double *)malloc(sizeof(double) * (size_t)sc->cols);
    if (!row_buf) {
        dst->free(dst);
        return NULL;
    }
    for (int r = 0; r < si->constraint_count; r++) {
        for (int c = 0; c < sc->cols; c++) {
            row_buf[c] = sc->data[r][c];
        }
        dst->add_constraint(dst, row_buf, sb->data[r]);
    }
    free(row_buf);

    return dst;
}

/**
 * @brief Clone a problem and apply a variable lower bound via substitution.
 */
static lp_problem *bc_clone_with_lower_bound(lp_problem *src, int var_idx,
                                             double lb, double *obj_offset) {
    lp_problem_impl *si = lp_problem_impl_from_public(src);
    if (!si) {
        return NULL;
    }

    matrix_impl *sc = matrix_impl_from_matrix(si->constraints);
    vector_impl *so = vector_impl_from_vector(si->objective);
    vector_impl *sb = vector_impl_from_vector(si->bounds);
    if (!sc || !so || !sb) {
        return NULL;
    }

    lp_problem *dst = create_problem(si->variable_count,
                                     si->constraint_count > 0 ? si->constraint_count + 2 : 2);
    if (!dst) {
        return NULL;
    }

    double *obj_buf = (double *)malloc(sizeof(double) * (size_t)si->variable_count);
    if (!obj_buf) {
        dst->free(dst);
        return NULL;
    }
    for (int i = 0; i < so->size; i++) {
        obj_buf[i] = so->data[i];
    }
    dst->set_objective(dst, obj_buf, si->type);
    *obj_offset = obj_buf[var_idx] * lb;
    free(obj_buf);

    double *row_buf = (double *)malloc(sizeof(double) * (size_t)sc->cols);
    if (!row_buf) {
        dst->free(dst);
        return NULL;
    }
    for (int r = 0; r < si->constraint_count; r++) {
        for (int c = 0; c < sc->cols; c++) {
            row_buf[c] = sc->data[r][c];
        }
        double adjusted_bound = sb->data[r] - sc->data[r][var_idx] * lb;
        dst->add_constraint(dst, row_buf, adjusted_bound);
    }
    free(row_buf);

    return dst;
}

/**
 * @brief Find the most fractional variable in the solution.
 */
static int bc_find_branching_variable(const double *solution, int num_vars) {
    int best_idx = -1;
    double best_frac = 0.0;

    for (int i = 0; i < num_vars; i++) {
        double frac = solution[i] - floor(solution[i]);
        double dist = frac > 0.5 ? 1.0 - frac : frac;
        if (dist > BC_INT_TOL && dist > best_frac) {
            best_frac = dist;
            best_idx = i;
        }
    }
    return best_idx;
}

/**
 * @brief Check whether the entire solution is integer-valued.
 */
static int bc_solution_is_integer(const double *solution, int num_vars) {
    for (int i = 0; i < num_vars; i++) {
        if (!is_integer(solution[i])) {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief Generate Gomory fractional cuts to tighten the LP relaxation.
 *
 * @param problem  LP problem to add cuts to.
 * @param tableau  Final simplex tableau from the LP solve.
 * @param num_vars Number of original decision variables.
 * @return Number of cuts added.
 */
static int add_gomory_cuts(lp_problem *problem, matrix *tableau, int num_vars) {
    matrix_impl *tab = matrix_impl_from_matrix(tableau);
    lp_problem_impl *pi = lp_problem_impl_from_public(problem);
    if (!tab || !pi) {
        return 0;
    }

    int cuts_added = 0;
    int rows = tab->rows - 1;
    int cols = tab->cols;

    double *cut_row = (double *)calloc((size_t)num_vars, sizeof(double));
    if (!cut_row) {
        return 0;
    }

    for (int i = 0; i < rows && cuts_added < BC_MAX_CUTS; i++) {
        double rhs = tab->data[i][cols - 1];
        double f0 = rhs - floor(rhs);

        if (f0 < BC_INT_TOL || f0 > 1.0 - BC_INT_TOL) {
            continue;
        }

        memset(cut_row, 0, sizeof(double) * (size_t)num_vars);

        int valid_cut = 1;
        for (int j = 0; j < num_vars && j < cols - 1; j++) {
            double aij = tab->data[i][j];
            double fij = aij - floor(aij);
            if (fij < -BC_COEFF_LIMIT || fij > BC_COEFF_LIMIT) {
                valid_cut = 0;
                break;
            }
            cut_row[j] = fij;
        }

        if (valid_cut && f0 > BC_INT_TOL) {
            problem->add_constraint(problem, cut_row, f0);
            cuts_added++;
        }
    }

    free(cut_row);
    return cuts_added;
}

/**
 * @brief Check whether a candidate solution is feasible for the original problem.
 */
static int bc_check_original_feasibility(const double *solution, int num_vars,
                                         const double *const *constraint_data,
                                         const double *bounds_data,
                                         int num_constraints) {
    for (int i = 0; i < num_constraints; i++) {
        double lhs = 0.0;
        for (int j = 0; j < num_vars; j++) {
            lhs += constraint_data[i][j] * solution[j];
        }
        if (lhs > bounds_data[i] + BC_INT_TOL) {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief Recursive branch-and-cut core.
 */
static void bc_solve(lp_problem *problem, double *best_obj, double *best_sol,
                     int num_vars, int is_max, int *nodes,
                     const double *offsets, double obj_offset,
                     const double *const *orig_constraints,
                     const double *orig_bounds, int orig_num_constr) {
    if (*nodes >= BC_MAX_NODES) {
        return;
    }
    (*nodes)++;

    /* Cutting plane loop */
    lp_problem *cut_problem = bc_clone_problem(problem);
    if (!cut_problem) {
        return;
    }

    matrix *tableau = NULL;
    int status = INFEASIBLE;
    int cut_rounds = 0;

    while (cut_rounds < BC_MAX_CUTS) {
        if (tableau) {
            tableau->free(tableau);
            tableau = NULL;
        }

        solver *round_solver = create_solver(SOLVER_SIMPLEX);
        if (!round_solver) {
            break;
        }

        status = round_solver->solve(round_solver, cut_problem, &tableau);
        round_solver->free(round_solver);

        if (status != OPTIMAL) {
            break;
        }

        lp_problem_impl *ci = lp_problem_impl_from_public(cut_problem);
        if (!ci || !ci->solution) {
            break;
        }

        /* Reconstruct true solution to check integrality */
        int all_integer = 1;
        for (int i = 0; i < num_vars; i++) {
            if (!is_integer(ci->solution[i] + offsets[i])) {
                all_integer = 0;
                break;
            }
        }
        if (all_integer) {
            break;
        }

        int cuts = 0;
        if (tableau) {
            cuts = add_gomory_cuts(cut_problem, tableau, num_vars);
        }
        if (cuts == 0) {
            break;
        }
        cut_rounds++;
    }

    if (status != OPTIMAL) {
        if (tableau) {
            tableau->free(tableau);
        }
        cut_problem->free(cut_problem);
        return;
    }

    lp_problem_impl *ci = lp_problem_impl_from_public(cut_problem);
    if (!ci || !ci->solution) {
        if (tableau) {
            tableau->free(tableau);
        }
        cut_problem->free(cut_problem);
        return;
    }

    double obj_val = ci->objective_value + obj_offset;

    /* Bound pruning */
    if (is_max && obj_val <= *best_obj && *best_obj > -DBL_MAX) {
        if (tableau) {
            tableau->free(tableau);
        }
        cut_problem->free(cut_problem);
        return;
    }
    if (!is_max && obj_val >= *best_obj && *best_obj < DBL_MAX) {
        if (tableau) {
            tableau->free(tableau);
        }
        cut_problem->free(cut_problem);
        return;
    }

    /* Reconstruct true solution */
    double *true_sol = (double *)malloc(sizeof(double) * (size_t)num_vars);
    if (!true_sol) {
        if (tableau) {
            tableau->free(tableau);
        }
        cut_problem->free(cut_problem);
        return;
    }
    for (int i = 0; i < num_vars; i++) {
        true_sol[i] = ci->solution[i] + offsets[i];
    }

    /* Check integrality and feasibility */
    if (bc_solution_is_integer(true_sol, num_vars) &&
        bc_check_original_feasibility(true_sol, num_vars,
                                      (const double *const *)orig_constraints,
                                      orig_bounds, orig_num_constr)) {
        int update = is_max ? (obj_val > *best_obj) : (obj_val < *best_obj);
        if (update) {
            *best_obj = obj_val;
            memcpy(best_sol, true_sol, sizeof(double) * (size_t)num_vars);
        }
        free(true_sol);
        if (tableau) {
            tableau->free(tableau);
        }
        cut_problem->free(cut_problem);
        return;
    }

    /* Branch on most fractional variable */
    int branch_var = bc_find_branching_variable(true_sol, num_vars);
    double branch_val = true_sol[branch_var];
    free(true_sol);

    if (tableau) {
        tableau->free(tableau);
    }
    cut_problem->free(cut_problem);

    if (branch_var < 0) {
        return;
    }

    /* Floor child: x[branch_var] <= floor(branch_val) */
    {
        lp_problem *child = bc_clone_problem(problem);
        if (child) {
            lp_problem_impl *chi = lp_problem_impl_from_public(child);
            matrix_impl *cc = chi ? matrix_impl_from_matrix(chi->constraints) : NULL;
            if (chi && cc) {
                double *row = (double *)calloc((size_t)cc->cols, sizeof(double));
                if (row) {
                    row[branch_var] = 1.0;
                    double ub = floor(branch_val) - offsets[branch_var];
                    if (ub >= 0.0) {
                        child->add_constraint(child, row, ub);
                        bc_solve(child, best_obj, best_sol, num_vars, is_max, nodes,
                                 offsets, obj_offset,
                                 orig_constraints, orig_bounds, orig_num_constr);
                    }
                    free(row);
                }
            }
            child->free(child);
        }
    }

    /* Ceil child via variable substitution */
    {
        double new_lb = ceil(branch_val);
        double sub_lb = new_lb - offsets[branch_var];
        if (sub_lb >= 0.0) {
            double child_obj_offset = 0.0;
            lp_problem *child = bc_clone_with_lower_bound(problem, branch_var,
                                                          sub_lb, &child_obj_offset);
            if (child) {
                double *new_offsets = (double *)malloc(sizeof(double) * (size_t)num_vars);
                if (new_offsets) {
                    memcpy(new_offsets, offsets, sizeof(double) * (size_t)num_vars);
                    new_offsets[branch_var] = new_lb;
                    bc_solve(child, best_obj, best_sol, num_vars, is_max, nodes,
                             new_offsets, obj_offset + child_obj_offset,
                             orig_constraints, orig_bounds, orig_num_constr);
                    free(new_offsets);
                }
                child->free(child);
            }
        }
    }
}

int branch_and_cut_solver(lp_problem *prob, matrix **out_tableau) {
    if (!out_tableau) {
        return INFEASIBLE;
    }
    *out_tableau = NULL;

    lp_problem_impl *impl = lp_problem_impl_from_public(prob);
    if (!impl) {
        return INFEASIBLE;
    }

    int num_vars = impl->variable_count;
    int is_max = (impl->type == PROBLEM_MAX) ? 1 : 0;

    lp_problem *root = bc_clone_problem(prob);
    if (!root) {
        return INFEASIBLE;
    }

    solver *lp_solver = create_solver(SOLVER_SIMPLEX);
    if (!lp_solver) {
        root->free(root);
        return INFEASIBLE;
    }

    matrix *root_tableau = NULL;
    int status = lp_solver->solve(lp_solver, root, &root_tableau);
    if (status != OPTIMAL) {
        if (root_tableau) {
            root_tableau->free(root_tableau);
        }
        lp_solver->free(lp_solver);
        root->free(root);
        return status;
    }

    lp_problem_impl *ri = lp_problem_impl_from_public(root);

    if (bc_solution_is_integer(ri->solution, num_vars)) {
        if (!impl->solution) {
            impl->solution = (double *)calloc((size_t)num_vars, sizeof(double));
        }
        if (impl->solution) {
            memcpy(impl->solution, ri->solution, sizeof(double) * (size_t)num_vars);
        }
        impl->objective_value = ri->objective_value;
        *out_tableau = root_tableau;
        lp_solver->free(lp_solver);
        root->free(root);
        return OPTIMAL;
    }

    if (root_tableau) {
        root_tableau->free(root_tableau);
    }
    lp_solver->free(lp_solver);

    double best_obj = is_max ? -DBL_MAX : DBL_MAX;
    double *best_sol = (double *)calloc((size_t)num_vars, sizeof(double));
    double *offsets = (double *)calloc((size_t)num_vars, sizeof(double));
    if (!best_sol || !offsets) {
        free(best_sol);
        free(offsets);
        root->free(root);
        return INFEASIBLE;
    }

    /* Extract original constraint data for feasibility checking */
    matrix_impl *orig_c = matrix_impl_from_matrix(impl->constraints);
    vector_impl *orig_b = vector_impl_from_vector(impl->bounds);

    int nodes = 0;
    bc_solve(root, &best_obj, best_sol, num_vars, is_max, &nodes, offsets, 0.0,
             (const double *const *)orig_c->data, orig_b->data, impl->constraint_count);
    root->free(root);
    free(offsets);

    if ((is_max && best_obj <= -DBL_MAX + 1.0) ||
        (!is_max && best_obj >= DBL_MAX - 1.0)) {
        free(best_sol);
        return INFEASIBLE;
    }

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
