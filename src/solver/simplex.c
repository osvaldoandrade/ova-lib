#include "simplex.h"
#include "../matrix/matrix_internal.h"

#include <math.h>
#include <stdlib.h>

int simplex_solver(lp_problem *prob, matrix **out_tableau) {
    if (!out_tableau) {
        return INFEASIBLE;
    }

    *out_tableau = NULL;

    lp_problem_impl *problem = lp_problem_impl_from_public(prob);
    matrix_impl *constraints = problem ? matrix_impl_from_matrix(problem->constraints) : NULL;
    vector_impl *objective = problem ? vector_impl_from_vector(problem->objective) : NULL;
    vector_impl *bounds = problem ? vector_impl_from_vector(problem->bounds) : NULL;
    if (!problem || !constraints || !objective || !bounds) {
        return INFEASIBLE;
    }

    int rows = problem->constraint_count;
    int cols = constraints->cols;
    if (rows <= 0 || cols <= 0) {
        return INFEASIBLE;
    }

    *out_tableau = create_matrix(rows + 1, cols + rows + 1);
    matrix_impl *tableau = matrix_impl_from_matrix(*out_tableau);
    if (!tableau) {
        return INFEASIBLE;
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            tableau->data[i][j] = constraints->data[i][j];
        }
        tableau->data[i][cols + i] = 1.0;
        tableau->data[i][tableau->cols - 1] = bounds->data[i];
    }

    for (int j = 0; j < cols; j++) {
        double coefficient = objective->data[j];
        tableau->data[rows][j] = (problem->type == PROBLEM_MIN) ? coefficient : -coefficient;
    }

    while (1) {
        int pivot_col = -1;
        int pivot_row = -1;
        double most_negative = -1e-9;
        double min_ratio = DBL_MAX;

        for (int j = 0; j < cols + rows; j++) {
            double value = tableau->data[rows][j];
            if (value < most_negative) {
                most_negative = value;
                pivot_col = j;
            }
        }

        if (pivot_col == -1) {
            if (!problem->solution) {
                problem->solution = (double *)calloc((size_t)cols, sizeof(double));
            }
            if (!problem->solution) {
                (*out_tableau)->free(*out_tableau);
                *out_tableau = NULL;
                return INFEASIBLE;
            }

            for (int i = 0; i < cols; i++) {
                problem->solution[i] = 0.0;
                for (int r = 0; r < rows; r++) {
                    if (fabs(tableau->data[r][i] - 1.0) < 1e-7) {
                        bool is_basic = true;
                        for (int rr = 0; rr < rows; rr++) {
                            if (rr != r && fabs(tableau->data[rr][i]) > 1e-7) {
                                is_basic = false;
                                break;
                            }
                        }
                        if (is_basic) {
                            problem->solution[i] = tableau->data[r][tableau->cols - 1];
                            break;
                        }
                    }
                }
            }

            problem->objective_value = tableau->data[rows][tableau->cols - 1];
            if (problem->type == PROBLEM_MIN) {
                problem->objective_value = -problem->objective_value;
            }
            return OPTIMAL;
        }

        for (int i = 0; i < rows; i++) {
            double pivot_candidate = tableau->data[i][pivot_col];
            if (pivot_candidate > 1e-9) {
                double ratio = tableau->data[i][tableau->cols - 1] / tableau->data[i][pivot_col];
                if (ratio >= 0.0 && ratio < min_ratio) {
                    min_ratio = ratio;
                    pivot_row = i;
                }
            }
        }

        if (pivot_row == -1) {
            return UNBOUNDED;
        }

        double pivot_value = tableau->data[pivot_row][pivot_col];
        if (fabs(pivot_value) < 1e-12) {
            (*out_tableau)->free(*out_tableau);
            *out_tableau = NULL;
            return INFEASIBLE;
        }

        for (int j = 0; j < tableau->cols; j++) {
            tableau->data[pivot_row][j] /= pivot_value;
        }

        for (int i = 0; i <= rows; i++) {
            if (i != pivot_row) {
                double factor = tableau->data[i][pivot_col];
                for (int j = 0; j < tableau->cols; j++) {
                    tableau->data[i][j] -= tableau->data[pivot_row][j] * factor;
                }
            }
        }
    }
}
